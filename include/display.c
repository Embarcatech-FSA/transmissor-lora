#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "display.h"
#include "config.h"

// Função para inicializar o objeto do display
void display_init(ssd1306_t *ssd) {
    // A inicialização do hardware I2C foi movida para o main
    ssd1306_init(ssd, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_I2C_ADDR, I2C1_PORT); // << Usa I2C1_PORT
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    printf("Display inicializado no I2C1.\n");
}

// ... (o restante das funções de display_update_screen, etc., permanecem as mesmas da resposta anterior) ...

void display_startup_screen(ssd1306_t *ssd) {
    ssd1306_fill(ssd, false);
    const char *line1 = "Transmissor";
    const char *line2 = "LoRa Sensor";
    uint8_t center_x = ssd->width / 2;
    uint8_t pos_x1 = center_x - (strlen(line1) * 8) / 2;
    uint8_t pos_x2 = center_x - (strlen(line2) * 8) / 2;
    ssd1306_draw_string(ssd, line1, pos_x1, 16);
    ssd1306_draw_string(ssd, line2, pos_x2, 36);
    ssd1306_send_data(ssd);
    sleep_ms(2000);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

void display_update_screen(ssd1306_t *ssd, DadosSistema_t *dados) {
    char buffer[32];
    ssd1306_fill(ssd, false);

    snprintf(buffer, sizeof(buffer), "LoRa: %s", dados->lora_ok ? "OK" : "Falha");
    ssd1306_draw_string(ssd, buffer, 2, 0);

    snprintf(buffer, sizeof(buffer), "T:%.1fC H:%.0f%%", dados->temperatura, dados->umidade);
    ssd1306_draw_string(ssd, buffer, 2, 16);

    snprintf(buffer, sizeof(buffer), "P: %.1f hPa", dados->pressao / 100.0f);
    ssd1306_draw_string(ssd, buffer, 2, 32);

    snprintf(buffer, sizeof(buffer), "Envios: %lu", dados->pacotes_enviados);
    ssd1306_draw_string(ssd, buffer, 2, 48);

    ssd1306_send_data(ssd);
}

void display_error_screen(ssd1306_t *ssd, const char *message) {
     ssd1306_fill(ssd, false);
     uint8_t center_x = ssd->width / 2;
     uint8_t pos_x = center_x - (strlen(message) * 8) / 2;
     ssd1306_draw_string(ssd, "ERRO:", center_x - (5 * 8 / 2), 16);
     ssd1306_draw_string(ssd, message, pos_x, 36);
     ssd1306_send_data(ssd);
}