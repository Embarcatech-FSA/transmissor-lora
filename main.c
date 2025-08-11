#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "include/config.h"
#include "dados.h"
#include "include/lora.h"
#include "include/external/aht20.h"
#include "include/external/bmp280.h"
#include "include/display.h"

// Declaração dos objetos globais
ssd1306_t display;
DadosSistema_t dados_sistema;

// Função para inicializar o barramento I2C0 para os sensores
void setup_i2c0_sensores() {
    i2c_init(I2C0_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);
    printf("I2C0 (Sensores) inicializado.\n");
}

// Função para inicializar o barramento I2C1 para o display
void setup_i2c1_display() {
    i2c_init(I2C1_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    printf("I2C1 (Display) inicializado.\n");
}


void setup_spi_lora() {
    // 1. Inicializa o periférico SPI na frequência desejada
    spi_init(LORA_SPI_PORT, 5 * 1000 * 1000); // 5 MHz

    // 2. Mapeia a função SPI para os pinos GPIO corretos
    gpio_set_function(LORA_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MISO_PIN, GPIO_FUNC_SPI);
    
    // 3. Os pinos CS, RST, e IRQ são GPIOs normais, inicializados separadamente
    // (A biblioteca lora.c já faz isso, então não precisamos repetir aqui)
    
    printf("SPI0 (LoRa) e pinos GPIO associados inicializados.\n");
}


int main() {
    // Inicializa E/S padrão para debug via USB
    stdio_init_all();
    sleep_ms(3000); // Aumenta o tempo para garantir que o monitor serial conecte

    // INICIALIZA TODOS OS PERIFÉRICOS DE HARDWARE PRIMEIRO
    printf("--- Iniciando Hardware ---\n");
    setup_i2c0_sensores();
    setup_i2c1_display();
    setup_spi_lora(); // <<< CHAMA A NOVA FUNÇÃO DE CONFIGURAÇÃO DO SPI
    printf("--------------------------\n\n");
    
    // Agora, inicializa os drivers dos dispositivos
    display_init(&display);
    display_startup_screen(&display);

    // Zera a estrutura de dados
    dados_sistema = (DadosSistema_t){0};

    // Inicializa sensores... (código sem alteração)
    if (!aht20_init() || !detect_bmp280_address() || !bmp280_init()) {
        printf("ERRO FATAL: Falha ao iniciar sensores.\n");
        display_error_screen(&display, "Sensor Falhou");
        while(1);
    }
    printf("Sensores AHT20 e BMP280 OK.\n");

    // Configurações do LoRa (agora usando macros do config.h)
    lora_config_t config = {
        .spi_port = LORA_SPI_PORT,
        .interrupt_pin = LORA_INTERRUPT_PIN,
        .cs_pin = LORA_CS_PIN,
        .reset_pin = LORA_RESET_PIN,
        .freq = LORA_FREQUENCY,         // <<< USA O PARÂMETRO DO CONFIG.H
        .tx_power = LORA_TX_POWER,      // <<< USA O PARÂMETRO DO CONFIG.H
        .this_address = LORA_ADDRESS_TRANSMITTER
    };

    // Inicializa LoRa
    if (lora_init(&config)) {
        printf("LoRa inicializado com sucesso!\n");
        dados_sistema.lora_ok = true;
    } else {
        printf("ERRO FATAL: Falha na inicializacao do LoRa.\n");
        dados_sistema.lora_ok = false;
        display_error_screen(&display, "LoRa Falhou");
    }

    while (1) {
        if (dados_sistema.lora_ok) {
            // Ler dados dos sensores
            aht20_read_data(&dados_sistema.temperatura, &dados_sistema.umidade);
            float temp_bmp;
            bmp280_read_data(&temp_bmp, &dados_sistema.pressao);

            printf("\n--- Novo Ciclo ---\n");
            printf("Dados Sensores: Temp=%.2fC, Umid=%.2f%%, Press=%.2f Pa\n", 
                dados_sistema.temperatura, dados_sistema.umidade, dados_sistema.pressao);

            // Atualizar o display com dados novos
            display_update_screen(&display, &dados_sistema);
            
            // Monta o payload para enviar
            char lora_buffer[64];
            snprintf(lora_buffer, sizeof(lora_buffer), "T:%.1f,H:%.0f,P:%.1f",
                     dados_sistema.temperatura, dados_sistema.umidade, dados_sistema.pressao / 100.0f);
            
            printf("Enviando pacote #%lu para o endereco #%d...\n", dados_sistema.pacotes_enviados + 1, LORA_ADDRESS_RECEIVER);
            printf("  Payload: \"%s\"\n", lora_buffer);

            // Envia dados e espera a conclusão
            lora_send((uint8_t*)lora_buffer, strlen(lora_buffer), LORA_ADDRESS_RECEIVER);
            
            dados_sistema.pacotes_enviados++;
            printf("Envio concluido.\n");
            
            // Atualiza o display novamente para mostrar o novo contador
            display_update_screen(&display, &dados_sistema);

        } else {
             printf("Sistema em estado de erro. Nao foi possivel enviar dados.\n");
        }

        sleep_ms(5000);
    }

    return 0;
}