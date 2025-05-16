/**
* @file    ILI9341_InitFunctions.c
 * @author  simim
 * @date    1. April 2025
 * @brief   Initialisierungsfunktionen für den ILI9341 TFT-Display-Controller
 *
 * Diese Datei enthält spezialisierte Funktionen für die Konfiguration und
 * Initialisierung des ILI9341 TFT-Display-Controllers. Jede Funktion implementiert
 * einen bestimmten Konfigurationsbefehl gemäß dem ILI9341-Datenblatt mit detaillierten
 * Parametereinstellungen für:
 *
 * - Power Control und Management
 * - Timing-Steuerung
 * - Spannungsreferenzen
 * - Gamma-Korrektur
 * - Display-Format und Arbeitsmodi
 *
 * Die Funktionen sind entsprechend des Initialisierungsablaufs organisiert und
 * vollständig dokumentiert, um die Wirkung jedes Parameters zu erläutern.
 *
 * Referenz: ILI9341 Datasheet - https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf
 */

#ifndef ILI9341_INITFUNCTIONS_H
#define ILI9341_INITFUNCTIONS_H

#include "main.h"
#include "ILI9341.h"

/**
 * @brief  Konfiguriert die Power Control A-Einstellungen für das ILI9341-Display.
 *
 * Diese Funktion sendet den Power Control A-Befehl (0xCB) mit spezifischen Parametern,
 * um die Stromversorgungseinstellungen des Displays zu konfigurieren:
 *
 * Parameter:
 * - 0x39: Erster Parameter für die Power Control A-Einstellungen
 * - 0x2C: Zweiter Parameter für die Power Control A-Einstellungen
 * - 0x00: Dritter Parameter für die Power Control A-Einstellungen
 * - 0x34: Vierter Parameter (00110100b)
 *         REG_VD[2:0] = 100b -> Stellt die Kernspannung (Vcore) auf 1.6V ein
 * - 0x02: Fünfter Parameter (00000010b)
 *         VBC[2:0] = 010b -> Stellt die DDVDH-Spannung auf 5.6V ein
 *
 */
void Power_Control_A() {
	uint8_t params[] = {
		0x39,
		0x2C,
		0x00,
		0x34, 	// 00110100b -> REG_VD[2:0] = 100b -> Vcore = 1.6V
		0x02	// 00000010b -> VBC[2:0] = 010b -> DDVDH(V) = 5.6
	};
	ILI9341_SendCommandWithParam_8Bit(0xCB,params,5);
}



/**
 * @brief  Konfiguriert die Power Control B-Einstellungen für das ILI9341-Display.
 *
 * Diese Funktion sendet den Power Control B-Befehl (0xCF) mit spezifischen Parametern,
 * um die Stromversorgungseinstellungen und Spannungspegel des Displays zu konfigurieren:
 *
 * Parameter:
 * - 0x00: Erster Parameter (reservierter Wert, gemäß Datenblatt)
 * - 0xC1: Zweiter Parameter (11000001b)
 *         Bit 7-6: Reserviert
 *         Bit 5: PCEQ = 1 -> PC und EQ-Operation für Energieeinsparung aktiviert
 *         Bit 4: DRV_ena = 1 -> VCOM-Treiberfähigkeit verstärkt
 *         Bit 3-2: Power control[1:0] = 00b -> Bestimmt VGH und VGL Spannungspegel
 *         Bit 1-0: Reserviert (01b)
 * - 0x30: Dritter Parameter (00110000b)
 *         Bit 7-6: DRV_vml[2:1] = 00b -> VML-Treibereinstellungen
 *         Bit 5-4: DC_ena = 11b -> Entladepfad für ESD-Schutz aktiviert
 *         Bit 3: DRV_vml[0] = 0b -> VML-Treibereinstellung
 *         Bit 2-0: DRV_vmh[2:0] = 000b -> VMH-Übertreibungsbreite auf 1 op_clk eingestellt
 *
 */
void Power_Control_B(void) {
    // Parameter für Power Control B-Befehl definieren
    const uint8_t params[3] = {
        0x00,   // Erster Parameter (reserviert)
        0xC1,   // Zweiter Parameter: PCEQ aktiv, DRV_ena aktiv
        0x30    // Dritter Parameter: DC_ena aktiv
    };

    // Befehl mit Parametern an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xCF, params, sizeof(params));
}


/**
 * @brief  Konfiguriert die Einschaltsequenz-Steuerung für das ILI9341-Display.
 *
 * Diese Funktion sendet den Power-On Sequence Control-Befehl (0xED) mit spezifischen Parametern,
 * um die Abfolge und Zeitsteuerung der Spannungsaktivierung beim Einschalten zu konfigurieren:
 *
 * Parameter:
 * - 0x64: Erster Parameter (01100100b)
 *         Bit 6: CP1 Soft Start = 1 -> Soft Start für 2 Frames aktiviert
 *         Bit 3: CP23 Soft Start = 0 -> Standard Soft Start für CP23
 *
 * - 0x03: Zweiter Parameter (00000011b)
 *         Bit 1: En_vcl = 1 -> VCL-Aktivierung im 2. Frame
 *         Bit 0: En_ddvdh = 1 -> DDVDH-Aktivierung im 2. Frame
 *
 * - 0x12: Dritter Parameter (00010010b)
 *         Bit 1: En_vgh = 1 -> VGH-Aktivierung im 3. Frame
 *         Bit 0: En_vgl = 0 -> VGL-Aktivierung im 1. Frame
 *
 * - 0x81: Vierter Parameter (10000001b)
 *         Bit 7: DDVDH_ENH = 1 -> DDVDH-Verstärkungsmodus aktiviert für 8 externe Kondensatoren
 *         Bit 0: Bit zur Aktivierung des DDVDH-Verstärkungsmodus
 *
 * Diese Konfiguration optimiert die Einschaltsequenz für Stabilität und Schutz der Display-Komponenten.
 */
//https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf#page=200
void Power_On_Sequence_Control(void) {
    // Parameter für die Einschaltsequenz-Steuerung definieren
    const uint8_t params[4] = {
        0x64,   // Erster Parameter: CP1 Soft Start für 2 Frames
        0x03,   // Zweiter Parameter: VCL und DDVDH im 2. Frame aktivieren
        0x12,   // Dritter Parameter: VGH im 3. Frame, VGL im 1. Frame aktivieren
        0x81    // Vierter Parameter: DDVDH-Verstärkungsmodus aktiviert
    };

    // Befehl mit Parametern an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xED, params, sizeof(params));
}

/**
 * @brief  Konfiguriert die Driver Timing Control A-Einstellungen für das ILI9341-Display.
 *
 * Diese Funktion sendet den Driver Timing Control A-Befehl (0xE8) mit spezifischen Parametern,
 * um die Timing-Einstellungen des Display-Treibers zu optimieren:
 *
 * Parameter:
 * - 0x85: Erster Parameter (10000101b)
 *         Bit 0: NOW = 1 -> Nicht-überlappende Timing-Steuerung des Gate-Treibers
 *                           auf "default non-overlap time + 1unit" eingestellt
 *
 * - 0x00: Zweiter Parameter (00000000b)
 *         Bit 4: EQ = 0 -> EQ-Timing-Steuerung auf "default - 1unit" eingestellt
 *         Bit 0: CR = 0 -> CR-Timing-Steuerung auf "default - 1unit" eingestellt
 *
 * - 0x78: Dritter Parameter (01111000b)
 *         Bit 6-5: Wert 11b -> Reserviert
 *         Bit 4-2: Wert 110b -> Reserviert
 *         Bit 1-0: PC[1:0] = 00b -> Vorlade-Timing auf "default - 2unit" eingestellt
 *
 */
//https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf#page=197
void DriverTimingControl_A(void) {
    // Parameter für Driver Timing Control A-Befehl definieren
    const uint8_t params[3] = {
        0x85,   // Erster Parameter: Gate-Treiber nicht-überlappende Zeit
        0x00,   // Zweiter Parameter: EQ- und CR-Timing-Steuerung
        0x78    // Dritter Parameter: Vorladezeit-Steuerung
    };

    // Befehl mit Parametern an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xE8, params, sizeof(params));
}


/**
 * @brief  Konfiguriert das Pumpenverhältnis (Pump Ratio Control) für das ILI9341-Display.
 *
 * Diese Funktion sendet den Pump Ratio Control-Befehl (0xF7) mit einem spezifischen Parameter,
 * um das Spannungsverhältnis der internen Ladungspumpe des Displays zu konfigurieren:
 *
 * Parameter:
 * - 0x20: (00100000b)
 *         Bit 5-4: Ratio[1:0] = 10b -> Stellt das DDVDH-Spannungsverhältnis auf 2xVCI ein
 *                  (00b und 01b sind reserviert, 11b würde DDVDH=3xVCI einstellen)
 *
 * Diese Einstellung ist wichtig für die korrekte Spannungsversorgung des Displays und
 * bestimmt die Effizienz sowie die Stabilität der Anzeigeoperationen.
 */
//https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf#page=202
void PumpRatioControl(void) {
    // Parameter für Pump Ratio Control-Befehl definieren
    const uint8_t params[1] = {0x20};   // DDVDH=2xVCI (Standardwert laut Datenblatt)

    // Befehl mit Parameter an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xF7, params, sizeof(params));
}

/**
 * @brief  Konfiguriert die Driver Timing Control B-Einstellungen für das ILI9341-Display.
 *
 * Diese Funktion sendet den Driver Timing Control B-Befehl (0xEA) mit spezifischen Parametern,
 * um die Timing-Einstellungen des Gate-Treibers zu konfigurieren:
 *
 * Parameter:
 * - 0x00: Erster Parameter (00000000b)
 *         Bit 7-6: VG_SW_T4[1:0] = 00b -> EQ zu GND: 0 Zeiteinheiten
 *         Bit 5-4: VG_SW_T3[1:0] = 00b -> EQ zu DDVDH: 0 Zeiteinheiten
 *         Bit 3-2: VG_SW_T2[1:0] = 00b -> EQ zu DDVDH: 0 Zeiteinheiten
 *         Bit 1-0: VG_SW_T1[1:0] = 00b -> EQ zu GND: 0 Zeiteinheiten
 *
 * - 0x00: Zweiter Parameter (00000000b)
 *         Bit 1-0: Reserviert, auf 0 gesetzt
 *
 * Diese Einstellungen beeinflussen das Timing des Gate-Treibers und
 * weichen vom Standardwert (0x66, 0x00) ab, was eine schnellere Schaltzeit
 * als Standard für alle Übergänge bedeutet.
 */
//https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf#page=197
void DriverTimingControl_B(void) {
    // Parameter für Driver Timing Control B-Befehl definieren
    const uint8_t params[2] = {
        0x00,   // Erster Parameter: Gate-Treiber-Timing (alle Übergänge: 0 Einheiten)
        0x00    // Zweiter Parameter: Reserviert
    };

    // Befehl mit Parametern an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xEA, params, sizeof(params));
}


/**
 * @brief  Konfiguriert Power Control 1-Einstellungen für das ILI9341-Display.
 *
 * Diese Funktion sendet den Power Control 1-Befehl (0xC0) mit einem spezifischen Parameter,
 * um den GVDD-Spannungspegel zu konfigurieren, der als Referenzwert für den VCOM-Pegel
 * und die Graustufenspannungspegel dient:
 *
 * Parameter:
 * - 0x23: (00100011b)
 *         Bits 5-0: VRH[5:0] = 100011b -> Stellt GVDD auf 4.60V ein
 *
 * Der GVDD-Wert ist entscheidend für die Bildqualität und den Stromverbrauch des Displays.
 * Gemäß Datenblatt muss GVDD ≤ (DDVDH - 0.2V) sein.
 *
 * @note Parameter 0x21  ist  der Standardwert nach Power-ON/Reset.
 */
//https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf#page=178
void PowerControl_1(void) {
    // Parameter für Power Control 1-Befehl definieren
    const uint8_t params[1] = {0x23};   // VRH[5:0] = 100011b -> GVDD = 4.60V

    // Befehl mit Parameter an das Display senden
    ILI9341_SendCommandWithParam_8Bit(0xC0, params, sizeof(params));
}

void PowerControl_2() {
	uint8_t params[] = {0x10};
	ILI9341_SendCommandWithParam_8Bit(0xC1,params,1);
}

void VCOM_Control_1() {
	uint8_t params[] = {0x3E,0x28};
	ILI9341_SendCommandWithParam_8Bit(0xC5,params,2);
}

void VCOM_Control_2() {
	uint8_t params[] = {0x86};
	ILI9341_SendCommandWithParam_8Bit(0xC7,params,1);
}

void MemoryAccessControl() {
	uint8_t params[] = {0x48};
	ILI9341_SendCommandWithParam_8Bit(0x36,params,1);
}

void VerticalScrollingStartAddress() {
	uint8_t params[] = {0x00};
	ILI9341_SendCommandWithParam_8Bit(0x37,params,1);
}

void COLMOD_PixelFormatSet() {
	//Set RGB Interface to 16bpp

	uint8_t params[] = {0x55};
	ILI9341_SendCommandWithParam_8Bit(0x3A,params,1);
}

void FrameRateControl() {
	uint8_t params[] = {0x00,0x18};
	ILI9341_SendCommandWithParam_8Bit(0xB1,params,2);
}

void DisplayFunctionControl() {
	uint8_t params[] = {0x08,0x82,0x27};
	ILI9341_SendCommandWithParam_8Bit(0xB6,params,3);
}

void Enable3G() {
	uint8_t params[] = {0x00};
	ILI9341_SendCommandWithParam_8Bit(0xF2,params,1);
}

void GammaSet() {
	uint8_t params[] = {0x01};
	ILI9341_SendCommandWithParam_8Bit(0x26,params,1);
}

void PositiveGammaCorrection() {
	uint8_t params[] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00};
	ILI9341_SendCommandWithParam_8Bit(0xE0,params,15);
}
void NegativeGammaCorrection(){
	uint8_t params[] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F};
	ILI9341_SendCommandWithParam_8Bit(0xE1,params,15);
}

void SleepOut() {
	ILI9341_SendCommand(0x11);
	HAL_Delay(120);
}

#endif //ILI9341_INITFUNCTIONS_H
