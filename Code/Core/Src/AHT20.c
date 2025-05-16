/*
 * AHT20.c
 *
 *  Created on: Mar 9, 2025
 *      Author: simim
 */

///
/// DATENBLATT AHT20
/// https://files.seeedstudio.com/wiki/Grove-AHT20_I2C_Industrial_Grade_Temperature_and_Humidity_Sensor/AHT20-datasheet-2020-4-16.pdf
///

#include "AHT20.h"

extern I2C_HandleTypeDef hi2c2;

/*
 * Adresse vom AHT20 Sensor ist 0x38. Aber da bei I2C das Addressenbyte immer [7:1] Addresse ;
 * [0] R/W (Lesen oder Schreiben Bit) wird die Adresse um 1
 * */
#define AHT_ADDR	0x38<<1
#define HI2C      hi2c2


/**
 * @brief Liest Temperatur und Luftfeuchtigkeit vom AHT20 Sensor über I2C.
 *
 * Diese Funktion kommuniziert mit dem AHT20 Sensor, um die aktuellen
 * Temperatur- und Feuchtigkeitswerte zu lesen. Sie prüft den Kalibrierungsstatus,
 * startet eine Messung und verarbeitet die Rohdaten, um die physikalischen Werte
 * zu berechnen.
 *
 * @param[out] Temp   Pointer zur Speicherung der Temperatur (in °C).
 * @param[out] Humid  Pointer zur Speicherung der Luftfeuchtigkeit (in % rF).
 */
void AHT20_Read(float* Temp, float* Humid)
{
    uint8_t dum[6]; // Puffer für I2C-Daten

    // 1. Statusregister auslesen (Adresse 0x71), um Kalibrierungsstatus zu prüfen
    HAL_I2C_Mem_Read(&HI2C, AHT_ADDR, 0x71, 1, dum, 1, 100);

    // 2. Kalibrierung durchführen, falls notwendig (Bit 3 = 0)
    if (!(dum[0] & (1 << 3)))
    {
        // Initialisierungsbefehl 0xBE mit Parametern 0x08 0x00 senden
        dum[0] = 0xBE, dum[1] = 0x08, dum[2] = 0x00;
        HAL_I2C_Master_Transmit(&HI2C, AHT_ADDR, dum, 3, 100);
        HAL_Delay(10); // Wartezeit für Kalibrierung
    }

    // 3. Messung starten (Command 0xAC mit Parametern 0x33 0x00)
    dum[0] = 0xAC, dum[1] = 0x33, dum[2] = 0x00;
    HAL_I2C_Master_Transmit(&HI2C, AHT_ADDR, dum, 3, 100);
    HAL_Delay(80); // Typische Messdauer laut Datenblatt

    // 4. Warten, bis Messung fertig (Bit 7 im Statusregister = 0)
    do {
        HAL_I2C_Mem_Read(&HI2C, AHT_ADDR, 0x71, 1, dum, 1, 100);
        HAL_Delay(1); // Kurze Wartezeit zwischen den Abfragen
    } while (dum[0] & (1 << 7)); // Wiederhole, solange Busy-Flag gesetzt

    // 5. Messdaten (6 Bytes) auslesen
    HAL_I2C_Master_Receive(&HI2C, AHT_ADDR, dum, 6, 100);

    /*
     * 6. Rohdaten zusammensetzen:
     * - Temperatur und Feuchtigkeit sind jeweils 20-Bit Werte
     * - Die Daten sind über mehrere Bytes verteilt
     */
    // Feuchtigkeitswert (20 Bit) aus Bytes 1-3 zusammensetzen
    uint32_t h20 = (dum[1]) << 12 | (dum[2]) << 4 | (dum[3] >> 4);

    // Temperaturwert (20 Bit) aus Bytes 3-5 zusammensetzen
    uint32_t t20 = (dum[3] & 0x0F) << 16 | (dum[4]) << 8 | dum[5];

    /*
     * 7. Rohwerte in physikalische Größen umrechnen
     * (Formeln laut AHT20 Datenblatt)
     */
    // Temperaturberechnung:
    // - 20-Bit Wert (0-1048575) -> 0...200 -> -50...+150°C
    *Temp = (t20 / 1048576.0) * 200.0 - 50.0;

    // Feuchtigkeitsberechnung:
    // - 20-Bit Wert (0-1048575) -> 0...100%
    *Humid = h20 / 10485.76; // 1048576/100 = 10485.76
}
