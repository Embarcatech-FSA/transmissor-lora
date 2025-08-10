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


int main() {
    // Inicializa E/S padrão para debug via USB
    stdio_init_all();
    sleep_ms(2000);

    // INICIALIZA OS DOIS BARRAMENTOS I2C
    setup_i2c0_sensores();
    setup_i2c1_display();
    
    // Inicializa Display (ele usará o I2C1 configurado)
    display_init(&display);
    display_startup_screen(&display);

    // Zera a estrutura de dados
    dados_sistema = (DadosSistema_t){0};

    // Inicializa Sensor AHT20 (ele usará o I2C0 por padrão)
    if (!aht20_init()) {
        printf("Falha ao inicializar AHT20\n");
        display_error_screen(&display, "AHT20 Falhou");
        while (1);
    }

    // Inicializa Sensor BMP280 (ele usará o I2C0 por padrão)
    if (!detect_bmp280_address() || !bmp280_init()) {
        printf("Falha ao inicializar BMP280\n");
        display_error_screen(&display, "BMP280 Falhou");
        while(1);
    }

    // Configurações do LoRa (usa os pinos definidos no config.h)
    lora_config_t config = {
        .spi_port = LORA_SPI_PORT,
        .interrupt_pin = LORA_INTERRUPT_PIN,
        .cs_pin = LORA_CS_PIN,
        .reset_pin = LORA_RESET_PIN,
        .freq = 868.0,
        .tx_power = 14,
        .this_address = LORA_ADDRESS_TRANSMITTER
    };

    // Inicializa LoRa
    if (lora_init(&config)) {
        printf("LoRa inicializado com sucesso!\n");
        dados_sistema.lora_ok = true;
    } else {
        printf("Falha na inicializacao do LoRa.\n");
        dados_sistema.lora_ok = false;
        display_error_screen(&display, "LoRa Falhou");
    }

    // O restante do loop while(1) permanece idêntico à resposta anterior...
    while (1) {
        aht20_read_data(&dados_sistema.temperatura, &dados_sistema.umidade);
        
        float temp_bmp;
        bmp280_read_data(&temp_bmp, &dados_sistema.pressao);

        printf("Temp: %.2fC, Umid: %.2f%%, Press: %.2f Pa\n", 
            dados_sistema.temperatura, dados_sistema.umidade, dados_sistema.pressao);

        display_update_screen(&display, &dados_sistema);

        if (dados_sistema.lora_ok) {
            char lora_buffer[64];
            snprintf(lora_buffer, sizeof(lora_buffer), "T:%.1f,H:%.0f,P:%.1f",
                     dados_sistema.temperatura, dados_sistema.umidade, dados_sistema.pressao / 100.0f);

            lora_send(lora_buffer, strlen(lora_buffer), LORA_ADDRESS_RECEIVER);
            
            dados_sistema.pacotes_enviados++;
            printf("Pacote #%lu enviado: %s\n", dados_sistema.pacotes_enviados, lora_buffer);
        }

        sleep_ms(5000);
    }

    return 0;
}