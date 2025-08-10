#include "pico/stdlib.h"
#include "include/communication/lora.h"
#include <stdio.h>

int main() {
    // Inicializa a E/S padrão
    stdio_init_all();

    // Configurações do LoRa
    lora_config_t config = {
        .spi_port = spi0,
        .interrupt_pin = 2,
        .cs_pin = 1,
        .reset_pin = 0,
        .freq = 868.0,
        .tx_power = 14,
        .this_address = 42
    };

    // Inicializa o LoRa
    if (lora_init(&config)) {
        printf("LoRa inicializado com sucesso!\n");
    } else {
        printf("Falha na inicialização do LoRa.\n");
        while (1);
    }

    // Envia um pacote
    char message[] = "Ola, LoRa!";
    lora_send(message, sizeof(message), 255); // Envia para o endereço de broadcast

    while (1) {
        // Loop principal da aplicação
    }

    return 0;
}