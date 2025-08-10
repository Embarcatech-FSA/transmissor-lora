#ifndef DISPLAY_H
#define DISPLAY_H

#include "dados.h" // Inclui a estrutura de estado 'DadosSistema_t'
#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/font.h" 

/**
 * @brief Inicializa o display OLED via I2C.
 * @param ssd Ponteiro para a instância do objeto ssd1306_t.
 */
void display_init(ssd1306_t *ssd);

/**
 * @brief Exibe uma tela de inicialização no display.
 * @param ssd Ponteiro para a instância do objeto ssd1306_t.
 */
void display_startup_screen(ssd1306_t *ssd);

/**
 * @brief Atualiza a tela com os dados atuais dos sensores e do sistema.
 * @param ssd Ponteiro para a instância do objeto ssd1306_t.
 * @param dados Ponteiro para a estrutura de dados do sistema.
 */
void display_update_screen(ssd1306_t *ssd, DadosSistema_t *dados);

/**
 * @brief Exibe uma mensagem de erro em tela cheia.
 * @param ssd Ponteiro para a instância do objeto ssd1306_t.
 * @param message Mensagem de erro a ser exibida.
 */
void display_error_screen(ssd1306_t *ssd, const char *message);

#endif // DISPLAY_H