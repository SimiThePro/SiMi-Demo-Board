//
// Created by simim on 14.04.2025.
//

#ifndef CLIONTEST_LED_H
#define CLIONTEST_LED_H

#include "Pin.h"

extern Pin LED_GREEN;
extern Pin LED_YELLOW;
extern Pin LED_RED;


void LED_Set(Pin pin, uint8_t state);
#endif //CLIONTEST_LED_H
