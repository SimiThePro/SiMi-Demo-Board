//
// Created by simim on 15.04.2025.
//

#include "Realtime.h"

#include <stdint.h>
#include <stdio.h>

#include "AHT20.h"
#include "ILI9341.h"
#include "SSD1306.h"
#include "stm32h7xx_hal.h"
#include "tim.h"
#include "Fonts/ssd1306_fonts.h"
/**************************************************************************************************
 *                                                                                                *
 *      Realtime Timer mit TIM7 - 1ms Interrupt-Erzeugung                                         *
 *                                                                                                *
 *      Systemtakt: Der Systemtakt (HAL_RCC_GetPCLK1Freq) wird durch die RCC (Reset and Clock     *
 *      Control) Konfiguration bestimmt und liefert die Grundfrequenz für den Timer.              *
 *                                                                                                *
 *      Timer-Konfiguration: TIM7 wird mit folgender Formel konfiguriert:                         *
 *      - Liest den CDPPRE1-Wert aus dem RCC-Register um zu bestimmen, ob der PCLK1 skaliert ist  *
 *      - Berechnet die tatsächliche Timer-Eingangsfrequenz (timer_clock_hz)                      *
 *      - Berechnet aus dieser Frequenz und dem Zielwert von 1000 Hz (1 ms) den Teilungsfaktor    *
 *        durch Kombination von Prescaler und Auto-Reload-Register (ARR)                          *
 *                                                                                                *
 *      Die Formel lautet: timer_clock_hz / (Prescaler+1) / (ARR+1) = 1000 Hz                     *
 *                                                                                                *
 *      Bei jedem Überlauf des Zählers (alle 1 ms) wird ein Interrupt ausgelöst, der die          *
 *      Funktion HAL_TIM_PeriodElapsedCallback() aufruft, welche wiederum die Realtime_Loop()     *
 *      Funktion ausführt.                                                                        *
 *                                                                                                *
 **************************************************************************************************/


void Realtime_Init(void) {
    // Timer 7 mit Interrupt starten
    HAL_TIM_Base_Start_IT(&htim7);
}

void Realtime_Loop(void) {
    // Diese Funktion wird exakt alle 1 ms durch den Timer 7 Interrupt aufgerufen
    static uint32_t ms_counter = 0;
    ms_counter++;

    // Beispiel für periodische Aktion jede Sekunde (1000 ms)
    if (ms_counter % 1000 == 0) {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    }

}
