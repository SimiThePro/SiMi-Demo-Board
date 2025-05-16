#include "WS2812.h"
#include <stdio.h>

/**
 * @file    WS2812.c
 * @author  simim
 * @date    13. März 2024
 * @brief   Treiber für WS2812B RGB-LEDs mit Timer/DMA-Ansteuerung
 *
 * Diese Implementierung bietet Funktionen zur Steuerung von WS2812B RGB-LEDs
 * (auch NeoPixel genannt) über PWM mit DMA-Unterstützung. Der Treiber nutzt
 * Timer1 zur präzisen Erzeugung der Timing-Signale und unterstützt:
 *
 * - Setzen von RGB-Farbwerten für LEDs
 * - Einstellung der Helligkeit mit Winkel-basierter Skalierung
 * - Effiziente Datenübertragung über DMA
 * - Blockierendes Senden mit Statusüberwachung
 *
 * Die Implementierung basiert auf dem präzisen Timing-Protokoll der WS2812B-LEDs,
 * bei dem Bits durch unterschiedliche Pulsbreiten dargestellt werden (1/3 vs 2/3
 * der Periodendauer für logisch 0 bzw. 1).
 *
 * Referenz: Worldsemi WS2812B Datenblatt - https://www.lcsc.com/datasheet/lcsc_datasheet_2504101957_Worldsemi-WS2812B-B-T_C2761795.pdf
 */

/**
 * @attention DMA-KONFIGURATION ERFORDERLICH
 *
 * Für die korrekte Funktionsweise dieses Treibers muss DMA für den verwendeten
 * Timer-Kanal (TIM1_CH1) konfiguriert und aktiviert sein. Folgende Einstellungen
 * sind notwendig:
 *
 * 1. DMA-Request: TIM1_CH1
 * 2. Datenrichtung: Memory-to-Peripheral
 * 3. Speicherinkrement: Aktiviert
 * 4. Periphere Datenbreite: Halbwort (16 Bit)
 * 5. Speicherdatenbreite: Halbwort (16 Bit)
 * 6. DMA-Priorität: Hoch oder Sehr hoch
 * 7. Circular Mode: Deaktiviert
 *
 * In STM32CubeMX:
 * - Aktiviere DMA für TIM1
 * - Konfiguriere einen DMA-Stream für TIM1_CH1
 * - Stelle die Parameter wie oben beschrieben ein
 *
 * Ohne korrekte DMA-Konfiguration können keine präzisen Timing-Signale
 * erzeugt werden, die für die WS2812-LEDs erforderlich sind.
 */


extern TIM_HandleTypeDef htim1;	//Variable für den Timer handle

uint8_t LED_Data[4];	//Buffer für Daten [0]:LEDIndex, [1] Grün, [2] Rot, [3] Blau
uint8_t LED_Mod[4];  	//Buffer f+r die Daten mit Helligkeit

uint16_t pwmData[(24*1)+50];	//Buffer mit Daten welche an die LED gesendet werden

volatile uint8_t datasentflag = 0;	//Flag für Kontrolle, ob die Datenübertragung abgeschlossen ist

uint16_t ARR_TIM1 = 0;	//Variable welche den Wert des AutoReload Register beinhaltet und beim Senden gesetzt wird

#define PI 3.14159265

/**
 * @brief Setzt die RGB-Werte für die LED.
 *
 * Diese Funktion speichert die übergebenen RGB-Werte in einem globalen Array,
 * das später für die Datenübertragung an die WS2812-LED verwendet wird.
 *
 * @param Red Der Rotwert der LED (0-255).
 * @param Green Der Grünwert der LED (0-255).
 * @param Blue Der Blauwert der LED (0-255).
 *
 * @note Der LED-Index wird standardmäßig auf 0 gesetzt.
 * @note Die Funktion verwendet das globale Array `LED_Data`.
 */
void WS2812_SetLED (int Red, int Green, int Blue)
{
	LED_Data[0] = 0;       // Setzt den LED-Index auf 0
	LED_Data[1] = Green;   // Speichert den Grünwert
	LED_Data[2] = Red;     // Speichert den Rotwert
	LED_Data[3] = Blue;    // Speichert den Blauwert
}


/**
 * @brief Setzt die Helligkeit der RGB-LED.
 *
 * Diese Funktion passt die Helligkeit der RGB-LED an, indem sie die RGB-Werte
 * entsprechend der angegebenen Helligkeit skaliert. Die Berechnung erfolgt
 * mithilfe eines Winkels, der in Abhängigkeit von der Helligkeit bestimmt wird.
 *
 * @param brightness Die gewünschte Helligkeit (0-45).
 *
 * @note Diese Funktion ist nur aktiv, wenn `USE_BRIGHTNESS` definiert ist.
 * @note Die maximale Helligkeit ist auf 45 begrenzt.
 * @note Die Funktion verwendet die globalen Arrays `LED_Data` und `LED_Mod`.
 */
void WS2812_SetBrightness (int brightness)  // 0-45
{
#if USE_BRIGHTNESS

    // Begrenze die Helligkeit auf den maximalen Wert von 45
    if (brightness > 45) brightness = 45;

    // Kopiere den LED-Index in das modifizierte Datenarray
    LED_Mod[0] = LED_Data[0];
    for (int j = 1; j < 4; j++)
    {
        // Berechne den Winkel basierend auf der Helligkeit (in Grad)
        float angle = 90 - brightness;
        angle = angle * PI / 180;  // Konvertiere den Winkel in Radiant

        // Skaliere die RGB-Werte basierend auf dem berechneten Winkel
        LED_Mod[j] = (LED_Data[j]) / (tan(angle));
    }

#endif

}

/**
 * @brief Sendet die aktuell gesetzten Farbdaten an die WS2812-RGB-LED
 *
 * Diese Funktion konvertiert die RGB-Farbdaten in das für WS2812-LEDs erforderliche
 * PWM-Signal-Format und sendet sie über DMA und Timer an die LED.
 *
 * Die Funktion führt folgende Schritte aus:
 * 1. Liest den aktuellen Timer-Periode-Wert (ARR) aus
 * 2. Kombiniert die RGB-Farbkomponenten (mit oder ohne Helligkeitsanpassung) in einen 32-Bit-Farbwert
 * 3. Konvertiert jeden der 24 Farbbit-Werte in entsprechende PWM-Pulsbreiten:
 *    - Logische '1': 2/3 der Periode
 *    - Logische '0': 1/3 der Periode
 * 4. Fügt eine Reset-Sequenz (50 Nullwerte) am Ende hinzu
 * 5. Sendet die Daten per DMA an den Timer-Kanal
 * 6. Wartet auf das Abschluss-Flag bevor die Funktion zurückkehrt
 *
 * @note Die Funktion verwendet die globalen Variablen LED_Data oder LED_Mod (je nach USE_BRIGHTNESS)
 * @note Das Senden erfolgt blockierend, die Funktion kehrt erst zurück, wenn der DMA-Transfer
 *       abgeschlossen ist (signalisiert durch datasentflag)
 *
 * @see WS2812_SetLED() zum Setzen der Farbwerte
 * @see WS2812_SetBrightness() zum Einstellen der Helligkeit
 * @see HAL_TIM_PWM_PulseFinishedCallback() wird aufgerufen, wenn die Übertragung abgeschlossen ist
 */
void WS2812_Send (void)
{
	//Lese das AutoReload Register aus
	ARR_TIM1 = htim1.Init.Period;

	// Falls aus irgendein Grund das Register nicht gesetzt ist
	if (ARR_TIM1 == 0) {
		printf("ARR has not been initialized");
		return;
	}

	uint32_t indx = 0;  // Index für das PWM-Datenarray
	uint32_t color;     // Variable für den kombinierten Farbwert

	// RGB-Daten mit oder ohne Helligkeitsanpassung kombinieren
#if USE_BRIGHTNESS
	color = ((LED_Mod[1] << 16) | (LED_Mod[2] << 8) | (LED_Mod[3]));
#else
	color = ((LED_Data[1] << 16) | (LED_Data[2] << 8) | (LED_Data[3]));
#endif

	// Konvertiere die 24 Farb-Bits in PWM-Daten
	for (int i = 23; i >= 0; i--) {
		if (color & (1 << i)) {
			// Logische '1': 2/3 der Periode
			pwmData[indx] = (uint16_t)((2.0 / 3.0) * (float)(ARR_TIM1 + 1));
		} else {
			// Logische '0': 1/3 der Periode
			pwmData[indx] = (uint16_t)((1.0 / 3.0) * (float)(ARR_TIM1 + 1));
		}
		indx++;
	}

	// Füge eine Reset-Sequenz (50 Nullwerte) hinzu
	for (int i = 0; i < 50; i++) {
		pwmData[indx] = 0;
		indx++;
	}

	uint32_t* address = (uint32_t*)pwmData;  // Adresse des PWM-Datenarrays

	HAL_Delay(1);  // Kurze Verzögerung

	// Starte den DMA-Transfer für den Timer-Kanal
	if (HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, address, 74) != HAL_OK) {
		Error_Handler();  // Fehlerbehandlung bei fehlgeschlagenem DMA-Start
	}

	// Warte, bis die Datenübertragung abgeschlossen ist
	while (!datasentflag) {};
	datasentflag = 0;  // Rücksetzen des Flags
}

/**
 * @brief Callback-Funktion, die aufgerufen wird, wenn ein PWM-Puls abgeschlossen ist
 *
 * Diese Funktion wird vom HAL-Timer-System automatisch aufgerufen, nachdem
 * ein PWM-Übertragungszyklus via DMA abgeschlossen wurde. Sie stoppt den
 * DMA-Transfer für den Timer-Kanal und setzt das Flag, welches anzeigt,
 * dass die Datenübertragung abgeschlossen ist.
 *
 * @param htim Zeiger auf die Timer-Handle-Struktur
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);  // Stoppt die DMA-Übertragung für Timer1 Kanal1
    datasentflag = 1;  // Setzt das Flag, das die Fertigstellung der Datenübertragung anzeigt
}
