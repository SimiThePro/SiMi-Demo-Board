/**
* @file LED_Matrix.c
 * @author simim
 * @brief Implementierung der Steuerung einer 8x8 LED-Matrix über MAX7219
 *
 * Diese Datei enthält die Implementierung für die Ansteuerung einer 8x8 LED-Matrix
 * mit dem MAX7219 LED-Treiber über die SPI-Schnittstelle (SPI4) eines STM32H7 Mikrocontrollers.
 * Die Funktionen ermöglichen die Grundkonfiguration, Helligkeitssteuerung und das Zeichnen
 * auf der Matrix durch direktes Setzen der einzelnen Zeilen.
 *
 * @note Die SPI-Taktfrequenz für den MAX7219 darf 10 MHz nicht überschreiten.
 * @see https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf
 */


#include "LED_Matrix.h"

#include <stdio.h>

#include "spi.h"
#include "stm32h7xx_it.h"

#define SPI_LED_Matrix &hspi4


/**
 * @brief Setzt das Chip-Select-Signal der LED-Matrix auf HIGH.
 *
 * Diese Funktion aktiviert das Chip-Select-Signal (CS) der LED-Matrix,
 * indem der entsprechende GPIO-Pin auf HIGH gesetzt wird. Dies wird
 * verwendet, um die Kommunikation mit der LED-Matrix zu beenden.
 */
void LEDM_CS_H(){
    HAL_GPIO_WritePin(LEDM_CS_GPIO_Port, LEDM_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Setzt das Chip-Select-Signal der LED-Matrix auf LOW.
 *
 * Diese Funktion deaktiviert das Chip-Select-Signal (CS) der LED-Matrix,
 * indem der entsprechende GPIO-Pin auf LOW gesetzt wird. Dies wird
 * verwendet, um die Kommunikation mit der LED-Matrix zu starten.
 */
void LEDM_CS_L(){
    HAL_GPIO_WritePin(LEDM_CS_GPIO_Port, LEDM_CS_Pin, GPIO_PIN_RESET);
}

/**
     * @brief Sendet einen Befehl an die LED-Matrix.
     *
     * Diese Funktion überträgt einen Befehl an die LED-Matrix, indem sie
     * die Adresse und die zugehörigen Daten über SPI sendet. Vor dem Senden
     * wird das Chip-Select-Signal aktiviert (LOW) und nach dem Senden
     * wieder deaktiviert (HIGH).
     *
     * @param address Die Register-Adresse auf der LED-Matrix, an die die Daten gesendet werden.
     * @param data Die zu sendenden Daten, die in das angegebene Register geschrieben werden.
     */
void LED_Matrix_send_command(uint8_t address,uint8_t data){
    //uint16_t send_data = (data<<8)|(address);
    uint8_t send_data[2] = {address,data};

    LEDM_CS_L();
    HAL_SPI_Transmit(SPI_LED_Matrix, send_data, 2, 100);
    LEDM_CS_H();
}


/**
 * @brief Initialisiert die LED-Matrix.
 *
 * Diese Funktion führt die grundlegende Konfiguration der LED-Matrix durch.
 * Sie setzt das Chip-Select-Signal, sendet initiale Daten über SPI und
 * konfiguriert die Matrix mit den entsprechenden Befehlen. Abschließend
 * wird die Matrix zurückgesetzt.
 * @note ACHTUNG: Die SPI-Baudrate darf nicht über 10 Mbit/s liegen,
 * @see https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf#page=3
 *
 */
void LED_Matrix_setup(){

    // ACHTUNG!!!!!
    // SPI BAUD RATE DARF NICHT ÜBER 10 MBIT/S SEIN, SONST FUNKTIONIERT DIE LED_MATRIX NICHT!!!!

    // Initialer Datenwert, der über SPI gesendet wird
    uint8_t data = 0;

    // Sendet den Wert 0, da die erste SPI Kommunikation nicht immer funktioniert. Somit wird versichert, dass die nächsten
    // SPI-Kommunikationen auch wirklich funktionieren
    LEDM_CS_L();
    HAL_SPI_Transmit(SPI_LED_Matrix, &data, 1, 100);
    LEDM_CS_H();

    // Setzt die LED-Matrix in den Shutdown-Modus
    LED_Matrix_set_mode(0);


    // Deaktiviert den Dekodierungsmodus
    //https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf#page=7
    LED_Matrix_send_command(0x09, 0x00);

    // Setzt die Scan-Limit-Register auf alle Zeilen
    //https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf#page=9
    LED_Matrix_send_command(0x0B, 0x07);

    // Setzt die Helligkeit der LED-Matrix
    LED_Matrix_set_intensity(2);

    // Setzt die LED-Matrix zurück
    LED_Matrix_reset();
}


/**
 * @brief Setzt den Betriebsmodus der LED-Matrix.
 *
 * Diese Funktion konfiguriert die LED-Matrix in den angegebenen Modus.
 * Der Modus wird durch das `mode`-Argument bestimmt:
 * - `0`: Shutdown-Modus (Matrix wird deaktiviert)
 * - `1`: Normaler Betriebsmodus (Matrix wird aktiviert)
 *
 * @param mode Der gewünschte Modus (0 für Shutdown, 1 für Normalbetrieb).
 */
void LED_Matrix_set_mode(uint8_t mode){

    uint16_t data = 0;

    if (mode == 1){	// Normalbetrieb
        data = 1;
    }
    LED_Matrix_send_command(0x0C, data);
}

/**
 * @brief Zeichnet eine Zeile auf der LED-Matrix.
 *
 * Diese Funktion schreibt Daten in eine bestimmte Zeile der LED-Matrix.
 * Die Zeile wird durch den Parameter `row` angegeben, und die Daten,
 * die in dieser Zeile angezeigt werden sollen, werden durch `data` definiert.
 *
 * @param row Die Zeilennummer (0-7), die beschrieben werden soll.
 *            Werte außerhalb dieses Bereichs werden ignoriert.
 * @param data Die anzuzeigenden Daten für die angegebene Zeile.
 *             Jeder Bitwert im Byte repräsentiert eine LED in der Zeile.
 */
void LED_Matrix_draw_row(uint8_t row, uint8_t data){
    if (row >= 8 || row < 0) return;

    uint8_t address = 0x01 + row;

    LED_Matrix_send_command(address, data);
}

/**
 * @brief Setzt die LED-Matrix zurück.
 *
 * Diese Funktion löscht alle Zeilen der LED-Matrix, indem sie die Daten
 * jeder Zeile auf 0 setzt. Anschließend wird die LED-Matrix in den
 * Normalbetrieb versetzt.
 */
void LED_Matrix_reset(){
    for (int i = 0; i < 8; i++){
        LED_Matrix_draw_row(i, 0);
    }
    LED_Matrix_set_mode(1);
}


/**
 * @brief Setzt die Intensität der LED-Matrix.
 *
 * Diese Funktion stellt die Helligkeit der LED-Matrix ein, indem sie
 * den Intensitätswert an das entsprechende Register sendet. Der Wert
 * wird auf den maximal zulässigen Bereich (0-15) begrenzt.
 *
 * @param intensity Die gewünschte Intensität (0-15), wobei 0 die niedrigste
 *                  und 15 die höchste Helligkeit darstellt.
 */
void LED_Matrix_set_intensity(uint8_t intensity) {

    if (intensity > 15) {
        intensity = 15; // Maximaler Wert für die Intensität
    }

    //https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf#page=9
    LED_Matrix_send_command(0x0A, intensity);
}

