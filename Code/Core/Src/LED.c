/**
 * @file LED.c
 * @brief Implementation der LED-Steuerungsfunktionen
 * @author simim
 *
 * Diese Datei implementiert Funktionen zur Steuerung der LEDs auf dem Board.
 * Sie definiert die Pin-Zuordnungen für grüne, gelbe und rote LEDs sowie
 * eine Funktion zum Setzen des LED-Zustands.
 */
#include "LED.h"
#include "main.h"


// LED_GREEN ist auf PD3
Pin LED_GREEN = {LED_GREEN_GPIO_Port, LED_GREEN_Pin};
// LED_YELLOW ist auf PD4
Pin LED_YELLOW = {LED_YELLOW_GPIO_Port, LED_YELLOW_Pin};
// LED_RED ist auf PD5
Pin LED_RED = {LED_RED_GPIO_Port, LED_RED_Pin};


/**
 * @brief Setzte den Status der LED
 * @param pin Pin struct welcher den GPIO-Port und Pin-Nummer enthält
 * @param state Status zum setzten (0 = Led aus, 1 = Led an)
 */
void LED_Set(Pin pin, uint8_t state) {
    HAL_GPIO_WritePin(pin.GPIOx,pin.GPIO_Pin,state);
}
