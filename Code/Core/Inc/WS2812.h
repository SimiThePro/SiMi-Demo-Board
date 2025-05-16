/*
 * WS2812.h
 *
 *  Created on: Apr 7, 2025
 *      Author: simim
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include "main.h"

#define MAX_LED 1
#define USE_BRIGHTNESS 1

extern uint8_t LED_Data[4];
extern uint8_t LED_Mod[4];  // for brightness

extern uint16_t pwmData[(24*1)+50];

void WS2812_SetLED(int Red, int Green, int Blue);
void WS2812_SetBrightness (int brightness);
void WS2812_Send (void);

#endif /* INC_WS2812_H_ */
