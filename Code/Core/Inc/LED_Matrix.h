//
// Created by simim on 14.04.2025.
//

#ifndef LED_MATRIX_H
#define LED_MATRIX_H
#include "main.h"

#define LED_MATRIX_SHUTDOWN_MODE 0
#define LED_MATRIX_NORMAL_OPERATION 1

void LED_Matrix_send_command(uint8_t address, uint8_t data);

void LED_Matrix_setup(void);

void LED_Matrix_set_mode(uint8_t mode);

void LED_Matrix_draw_row(uint8_t row, uint8_t data);

void LED_Matrix_reset(void);

void LED_Matrix_set_intensity(uint8_t intensity);

void LED_Matrix_draw_matrix(uint8_t matrix[8][8]);

#endif //LED_MATRIX_H
