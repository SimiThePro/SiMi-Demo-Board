//
// Created by simim on 14.04.2025.
//

#ifndef CLIONTEST_PIN_H
#define CLIONTEST_PIN_H

#include "main.h"

typedef struct Pin{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
}Pin;

#endif //CLIONTEST_PIN_H
