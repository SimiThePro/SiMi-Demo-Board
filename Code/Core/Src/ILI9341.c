/**
* @file    ILI9341.c
 * @author  simim
 * @date    15. September 2024
 * @brief   Treiber für das ILI9341 TFT-Display mit SPI-Schnittstelle
 *
 * Diese Implementierung bietet eine umfassende Bibliothek für die Ansteuerung
 * des ILI9341-Displays über SPI. Sie umfasst grundlegende und erweiterte
 * Grafikfunktionen wie:
 * - Initialisierung und Konfiguration des Displays
 * - Pixel-, Linien-, Rechteck- und Kreiszeichnung
 * - Text- und Bilddarstellung
 * - Verschiedene Füll- und Umrissoperationen
 * - Unterstützung für unterschiedliche Displayausrichtungen
 *
 * Referenz: Datasheet: https://www.adafruit.com/datasheets/ILI9341.pdf
 */

#include "main.h"
#include <ILI9341.h>
#include <math.h>

#include "ff.h"
#include "Fonts/5x5_font.h"
#include "stdio.h"
#include "ILI9341_InitFunctions.h"
#include "SDCard.h"

// Konstanten und globale Variablen
#define CHUNK_SIZE_IN  ((uint32_t)(64 * 1024))
#define CHUNK_SIZE_OUT ((uint32_t)(64 * 1024))

int16_t cursor_x, cursor_y;
uint16_t textcolor, textbgcolor;


SPI_HandleTypeDef* ILI9341_SPI;

GPIO_TypeDef* ILI9341_CS_Port;
uint16_t ILI9341_CS_Pin;

GPIO_TypeDef* ILI9341_DC_Port;
uint16_t ILI9341_DC_Pin;

GPIO_TypeDef* ILI9341_Reset_Port;
uint16_t ILI9341_Reset_Pin;

uint16_t ILI9341_WIDTH = 240;
uint16_t ILI9341_HEIGHT = 320;

#ifdef USE_JPEG_ENCODING
extern __IO uint32_t Jpeg_HWDecodingEnd;
#endif



void SecretCommand();

// Hilfsfunktionen
static uint32_t fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required)
{
uint32_t val;
uint8_t *s = (uint8_t *)&p[index>>3];

#ifdef UNALIGNED_IS_SAFE
	val = *(uint32_t *)s; // read 4 bytes - unaligned is ok
	val = __builtin_bswap32(val); // change to big-endian order
#else
	val = s[0] << 24;
	val |= (s[1] << 16);
	val |= (s[2] << 8);
	val |= s[3];
#endif
	val <<= (index & 7); // shift out used bits
	if (32 - (index & 7) < required) { // need to get more bits
		val |= (s[4] >> (8 - (index & 7)));
        }
	val >>= (32-required); // right align the bits
	return val;
}

static inline uint32_t fetchbit(const uint8_t *p, uint32_t index)
{
	return (p[index >> 3] & (0x80 >> (index & 7)));
}


// Funktionen zur Steuerung des Displays
/**
 * Aktiviert das Display, indem der CS-Pin auf LOW gesetzt wird.
 */
void ILI9341_ChipSelect() {
	HAL_GPIO_WritePin(ILI9341_CS_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}

/**
 * Deaktiviert das Display, indem der CS-Pin auf HIGH gesetzt wird.
 */
void ILI9341_ChipDeselect() {
	HAL_GPIO_WritePin(ILI9341_CS_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
 * Setzt das Display in den Befehlsmodus (D/CX-Pin auf LOW).
 */
void ILI9341_SetCommand() {
    HAL_GPIO_WritePin(ILI9341_DC_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
}

/**
 * Setzt das Display in den Datenmodus (D/CX-Pin auf HIGH).
 */
void ILI9341_SetData() {
	//When DCX = ’1’, data is selected
	/* If the D/CX bit is “high”, the transmission byte is stored as
the display data RAM (Memory write command), or command register as parameter*/
	HAL_GPIO_WritePin(ILI9341_DC_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
}

/**
 * Initialisiert das ILI9341-Display.
 */
/**
 * @brief  Initialisiert das ILI9341 Display
 * @param  DISPLAY_SPI:       SPI-Handler für die Kommunikation mit dem Display
 * @param  _ILI9341_CS_Port:  GPIO-Port für das Chip-Select-Signal
 * @param  _ILI9341_CS_Pin:   GPIO-Pin für das Chip-Select-Signal
 * @param  _ILI9341_DC_Port:  GPIO-Port für das Data/Command-Signal
 * @param  _ILI9341_DC_Pin:   GPIO-Pin für das Data/Command-Signal
 * @param  _ILI9341_Reset_Port: GPIO-Port für das Reset-Signal
 * @param  _ILI9341_Reset_Pin:  GPIO-Pin für das Reset-Signal
 * @retval None
 *
 * @note Diese Funktion führt folgende Initialisierungsschritte durch:
 *       1. Speichern der SPI- und GPIO-Konfigurationen
 *       2. Hardware-Reset des Displays
 *       3. SPI-Initialisierung
 *       4. Konfiguration der Display-Einstellungen
 *       5. Aktivierung des Displays
 *       6. Einstellung der Standard-Ausrichtung
 */
void ILI9341_begin(SPI_HandleTypeDef* DISPLAY_SPI, GPIO_TypeDef* _ILI9341_CS_Port,
                   uint16_t _ILI9341_CS_Pin, GPIO_TypeDef* _ILI9341_DC_Port,
                   uint16_t _ILI9341_DC_Pin, GPIO_TypeDef* _ILI9341_Reset_Port,
                   uint16_t _ILI9341_Reset_Pin)
{
    /* GPIO und SPI-Handler speichern */
    ILI9341_SPI = DISPLAY_SPI;
    ILI9341_CS_Port = _ILI9341_CS_Port;
    ILI9341_CS_Pin = _ILI9341_CS_Pin;
    ILI9341_DC_Port = _ILI9341_DC_Port;
    ILI9341_DC_Pin = _ILI9341_DC_Pin;
    ILI9341_Reset_Port = _ILI9341_Reset_Port;
    ILI9341_Reset_Pin = _ILI9341_Reset_Pin;

    /* Hardware-Reset durchführen */
    HAL_GPIO_WritePin(ILI9341_Reset_Port, ILI9341_Reset_Pin, GPIO_PIN_SET);
    ILI9341_ChipSelect();
    HAL_GPIO_WritePin(ILI9341_Reset_Port, ILI9341_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(200);  // Reset-Impuls für 200ms
    ILI9341_ChipDeselect();
    HAL_Delay(200);  // Warten nach Reset
    HAL_GPIO_WritePin(ILI9341_Reset_Port, ILI9341_Reset_Pin, GPIO_PIN_SET);

    /* SPI-Interface mit Dummy-Byte initialisieren */
    uint8_t dummy_byte = 0b01010101;
    ILI9341_ChipSelect();
    HAL_SPI_Transmit(ILI9341_SPI, &dummy_byte, 1, 100);
    ILI9341_ChipDeselect();

    /* Software-Reset senden */
    ILI9341_SendCommand(0x01);  // Software Reset Kommando
    HAL_Delay(150);             // Warten auf Reset-Abschluss

    /* Display-Spezifische Einstellungen konfigurieren */
    // Power-Management und Timing konfigurieren
    Power_Control_A();          // Leistungssteuerung A
    Power_Control_B();          // Leistungssteuerung B
    DriverTimingControl_A();    // Treiber-Timing A
    DriverTimingControl_B();    // Treiber-Timing B
    Power_On_Sequence_Control(); // Einschaltsequenz-Steuerung
    PumpRatioControl();         // Pumpen-Verhältnis-Steuerung

    // Spannung und Stromversorgung konfigurieren
    PowerControl_1();           // Leistungssteuerung 1
    PowerControl_2();           // Leistungssteuerung 2
    VCOM_Control_1();           // VCOM-Steuerung 1
    VCOM_Control_2();           // VCOM-Steuerung 2

    // Display-Funktionen konfigurieren
    MemoryAccessControl();      // Speicherzugriffsteuerung
    COLMOD_PixelFormatSet();    // Pixelformat einstellen (16 Bit pro Pixel)
    FrameRateControl();         // Bildwiederholrate einstellen
    DisplayFunctionControl();   // Display-Funktionssteuerung
    Enable3G();                 // 3G-Funktion aktivieren

    // Gamma-Korrektur konfigurieren
    GammaSet();                 // Gamma-Kurve auswählen
    PositiveGammaCorrection();  // Positive Gamma-Korrektur
    NegativeGammaCorrection();  // Negative Gamma-Korrektur

    // Display aktivieren
    SleepOut();                 // Sleep-Modus beenden
    ILI9341_DisplayOn();        // Display einschalten

    /* Standard-Ausrichtung einstellen */
    ILI9341_SetRotation(SCREEN_VERTICAL_2);  // Portrait-Modus (180° gedreht)
}


/**
 * @brief  Sendet einen Befehl an das ILI9341-Display über die SPI-Schnittstelle.
 *
 * Diese Funktion wählt das Display aus, setzt es in den Befehlsmodus,
 * überträgt das Befehlsbyte und hebt anschließend die Auswahl des Displays auf.
 *
 * @param  cmd Das Befehlsbyte, das an das Display gesendet werden soll.
 * @retval HAL_StatusTypeDef Der Status der SPI-Übertragung (HAL_OK bei Erfolg,
 *         andernfalls ein Fehlercode).
 */
HAL_StatusTypeDef ILI9341_SendCommand(uint8_t cmd) {

	ILI9341_ChipSelect(); // Wählt das Display aus
	ILI9341_SetCommand(); // Setzt den Modus auf "Befehl"

	HAL_StatusTypeDef status;
	status = HAL_SPI_Transmit(ILI9341_SPI, &cmd, 1, 100); // Überträgt das Befehlsbyte

	ILI9341_ChipDeselect(); // Hebt die Auswahl des Displays auf

	return status; // Gibt den Status der Übertragung zurück
}

/**
 * @brief  Empfängt ein einzelnes Byte vom ILI9341-Display über die SPI-Schnittstelle.
 *
 * Diese Funktion wählt das Display aus, empfängt ein Byte und hebt anschließend die Auswahl des Displays auf.
 * Das empfangene Byte wird zurückgegeben.
 *
 * @retval uint8_t Das empfangene Byte vom Display.
 */
uint8_t ILI9341_ReceiveByte() {
	ILI9341_ChipSelect(); // Wählt das Display aus
	uint8_t data;

	HAL_SPI_Receive(ILI9341_SPI, &data, 1, 100); // Empfängt ein Byte über SPI

	ILI9341_ChipDeselect(); // Hebt die Auswahl des Displays auf

	return data; // Gibt das empfangene Byte zurück
}

/**
 * @brief  Empfängt mehrere Bytes vom ILI9341-Display über die SPI-Schnittstelle.
 *
 * Diese Funktion wählt das Display aus, empfängt eine angegebene Anzahl von Bytes
 * und hebt anschließend die Auswahl des Displays auf. Der Status der SPI-Übertragung
 * wird zurückgegeben.
 *
 * @param  dataOut Zeiger auf den Puffer, in dem die empfangenen Daten gespeichert werden.
 * @param  pSize   Anzahl der zu empfangenden Bytes.
 * @retval HAL_StatusTypeDef Status der SPI-Übertragung (HAL_OK bei Erfolg,
 *         andernfalls ein Fehlercode).
 */
HAL_StatusTypeDef ILI9341_ReceiveData(uint8_t *dataOut, uint8_t pSize) {
	// Wählt das Display aus, indem der CS-Pin auf LOW gesetzt wird
	ILI9341_ChipSelect();

	// Lokaler Puffer für die empfangenen Daten
	uint8_t data[pSize];

	// Führt die SPI-Datenübertragung aus und speichert den Status
	HAL_StatusTypeDef status;
	status = HAL_SPI_Receive(ILI9341_SPI, data, pSize, HAL_MAX_DELAY);

	// Hebt die Auswahl des Displays auf, indem der CS-Pin auf HIGH gesetzt wird
	ILI9341_ChipDeselect();

	// Gibt den Status der SPI-Übertragung zurück
	return status;
}

/**
  * @brief  Sendet einen Befehl an das ILI9341-Display und empfängt anschließend mehrere Datenbytes.
  *
  * Diese Funktion führt folgende Schritte aus:
  * 1. Sendet ein Befehlsbyte an das Display über `ILI9341_SendCommand`.
  * 2. Schaltet das Display in den Datenmodus mit `ILI9341_SetData`.
  * 3. Empfängt die angegebene Anzahl von Datenbytes mit `ILI9341_ReceiveData`.
  *
  * @param  cmd     Das Befehlsbyte, das an das Display gesendet werden soll.
  * @param  dataOut Zeiger auf den Puffer, in dem die empfangenen Daten gespeichert werden.
  * @param  pSize   Anzahl der zu empfangenden Bytes.
  * @retval HAL_StatusTypeDef Status der Operation:
  *         - HAL_OK: Operation erfolgreich ausgeführt.
  *         - Fehlercode: Wenn die Operation fehlschlägt.
  */
 HAL_StatusTypeDef ILI9341_SendCommandAndReceive(uint8_t cmd, uint8_t *dataOut,
 		uint8_t pSize) {

 	HAL_StatusTypeDef status;
 	status = ILI9341_SendCommand(cmd);

 	ILI9341_SetData();

 	status = ILI9341_ReceiveData(dataOut, pSize);

 	return status;
 }



/**
 * @brief  Sendet einen Befehl an das ILI9341-Display gefolgt von 8-Bit-Parametern über die SPI-Schnittstelle.
 *
 * Diese Funktion führt folgende Schritte aus:
 * 1. Wählt das Display aus (CS-Pin auf LOW).
 * 2. Setzt den Kommunikationsmodus auf Befehlsmodus (D/CX-Pin auf LOW).
 * 3. Überträgt das Befehlsbyte an das Display.
 * 4. Wechselt in den Datenmodus (D/CX-Pin auf HIGH).
 * 5. Überträgt die angegebenen Parameter an das Display.
 * 6. Deaktiviert die Auswahl des Displays (CS-Pin auf HIGH).
 *
 * @param  cmd    Das Befehlsbyte, das an das Display gesendet werden soll.
 * @param  Params Zeiger auf den Puffer mit den 8-Bit-Parametern, die gesendet werden sollen.
 * @param  pSize  Anzahl der zu sendenden Parameter (Bytes).
 * @retval HAL_StatusTypeDef Status der Operation (HAL_OK bei Erfolg, andernfalls ein Fehlercode).
 */
HAL_StatusTypeDef ILI9341_SendCommandWithParam_8Bit(uint8_t cmd, uint8_t *Params,
		uint8_t pSize) {
	ILI9341_ChipSelect();

	ILI9341_SetCommand();

	HAL_StatusTypeDef status;
	status = HAL_SPI_Transmit(ILI9341_SPI, &cmd, 1, 100);

	ILI9341_SetData();

	status = HAL_SPI_Transmit(ILI9341_SPI, Params, pSize, HAL_MAX_DELAY);

	ILI9341_ChipDeselect();

	return status;
}

/**
 * @brief  Sendet einen Befehl an das ILI9341-Display gefolgt von 16-Bit-Parametern über die SPI-Schnittstelle.
 *
 * Diese Funktion führt folgende Schritte aus:
 * 1. Konvertiert 16-Bit-Parameter in 8-Bit-Daten für die Übertragung
 * 2. Wählt das Display aus (CS-Pin auf LOW)
 * 3. Setzt den Kommunikationsmodus auf Befehlsmodus (D/CX-Pin auf LOW)
 * 4. Sendet das Befehlsbyte an das Display
 * 5. Wechselt in den Datenmodus (D/CX-Pin auf HIGH)
 * 6. Überträgt die konvertierten Parameter
 * 7. Deaktiviert die Auswahl des Displays (CS-Pin auf HIGH)
 *
 * @param  cmd    Das Befehlsbyte, das an das Display gesendet werden soll.
 * @param  Params Zeiger auf den Puffer mit den 16-Bit-Parametern, die gesendet werden sollen.
 * @param  pSize  Anzahl der zu sendenden Bytes (muss gerade sein für 16-Bit-Parameter).
 * @retval HAL_StatusTypeDef Status der Operation (HAL_OK bei Erfolg, andernfalls ein Fehlercode).
 */
HAL_StatusTypeDef ILI9341_SendCommandWithParam_16Bit(uint8_t cmd, uint16_t* Params, uint8_t pSize){


/**
 * Diese Schleife durchläuft die 16-Bit-Parameter und teilt sie in zwei 8-Bit-Werte auf,
 * die in einem neuen Array gespeichert werden. Der erste 8-Bit-Wert wird in das höhere Byte
 * und der zweite in das niedrigere Byte des Zielarrays geschrieben.
 */
	uint8_t txData[pSize];
	int j = 0;
	for (int i = 0; i < pSize; i+=2){
		txData[i] = Params[j] << 8;
		txData[i+1] = Params[j];
		j++;
	}


	ILI9341_ChipSelect();

	ILI9341_SetCommand();

	HAL_StatusTypeDef status;

	status = HAL_SPI_Transmit(ILI9341_SPI, &cmd, 1, 100);

	ILI9341_SetData();

	status = HAL_SPI_Transmit(ILI9341_SPI, txData, pSize, HAL_MAX_DELAY);


	ILI9341_ChipDeselect();

	return status;
}


/**
 * @brief  Legt den Spaltenadressbereich für das ILI9341-Display fest.
 *
 * Diese Funktion sendet den Spaltenadressbefehl (0x2A), gefolgt von den Start- (SC)
 * und End- (EC) Spaltenadressen. Die Adressen werden als 16-Bit-Parameter gesendet.
 *
 * @param  SC Startadresse der Spalte.
 * @param  EC Endadresse der Spalte.
 */
void ILI9341_ColumnAddressSet(uint16_t SC, uint16_t EC) {

	uint16_t txData[2] = {SC, EC};

	ILI9341_SendCommandWithParam_16Bit(0x2A, txData, 4);
}

/**
  * @brief  Legt den Zeilenadressbereich für das ILI9341-Display fest.
  *
  * Diese Funktion sendet den Zeilenadressbefehl (0x2B), gefolgt von den Start- (SC)
  * und End- (EC) Zeilenadressen. Die Adressen werden als 16-Bit-Parameter gesendet.
  *
  * @param  SC Startadresse der Zeile.
  * @param  EC Endadresse der Zeile.
  */
 void ILI9341_RowAddressSet(uint16_t SC, uint16_t EC) {
 	uint16_t txData[2] = {SC, EC};

 	ILI9341_SendCommandWithParam_16Bit(0x2B, txData, 4);
 }

/**
  * @brief  Sendet Daten an das ILI9341-Display über die SPI-Schnittstelle.
  *
  * Diese Funktion führt folgende Schritte aus:
  * 1. Wählt das Display aus, indem der CS-Pin auf LOW gesetzt wird.
  * 2. Setzt das Display in den Datenmodus (D/CX-Pin auf HIGH).
  * 3. Überträgt die bereitgestellten Daten über die SPI-Schnittstelle.
  * 4. Hebt die Auswahl des Displays auf, indem der CS-Pin auf HIGH gesetzt wird.
  *
  * @param  Data  Zeiger auf den Puffer, der die zu sendenden Daten enthält.
  * @param  pSize Anzahl der zu sendenden Bytes.
  * @retval HAL_StatusTypeDef Status der SPI-Übertragung:
  *         - HAL_OK: Übertragung erfolgreich.
  *         - Fehlercode: Wenn die Übertragung fehlschlägt.
  */
 HAL_StatusTypeDef ILI9341_SendData(uint8_t *Data, uint32_t pSize) {
 	ILI9341_ChipSelect();

 	HAL_StatusTypeDef status;

 	ILI9341_SetData();

 	status = HAL_SPI_Transmit(ILI9341_SPI, Data, pSize, HAL_MAX_DELAY);

 	ILI9341_ChipDeselect();

 	return status;
 }


/**
 * @brief  Schreibt Rohdaten direkt in den Speicher des ILI9341-Displays.
 *
 * Diese Funktion sendet zuerst den Speicherschreibbefehl (0x2C) an das Display
 * und überträgt dann die bereitgestellten Rohdaten. Sie wird verwendet, um
 * Pixeldaten oder andere Rohdaten direkt in den Anzeigespeicher zu schreiben,
 * ohne zusätzliche Formatierungen oder Adressierungen vorzunehmen.
 *
 * Der Prozess besteht aus zwei Hauptschritten:
 * 1. Senden des Memory Write-Befehls (0x2C)
 * 2. Übertragen der angegebenen Daten über die SPI-Schnittstelle
 *
 * @param  data Zeiger auf den Puffer, der die zu schreibenden Daten enthält.
 * @param  size Anzahl der zu schreibenden Bytes.
 * @note   Vor dem Aufruf dieser Funktion sollte der Adressbereich mit
 *         ILI9341_SetAddress() oder ähnlichen Funktionen festgelegt werden,
 *         um die korrekte Position auf dem Display zu definieren.
 */
void ILI9341_MemoryWriteRaw(uint8_t *data, uint32_t size){
	ILI9341_SendCommand(0x2C);
	ILI9341_SendData(data, size);
}

void ILI9341_DisplayOn() {
	ILI9341_SendCommand(0x29);
	HAL_Delay(10);
}

/**
 * @brief  Legt das Adressfenster für das ILI9341-Display fest.
 *
 * Diese Funktion definiert den rechteckigen Bereich für das Schreiben oder Lesen von Pixeldaten,
 * indem sie die Spalten- und Zeilenadressbereiche festlegt. Sie sendet die Befehle
 * "Column Address Set" (0x2A) und "Row Address Set" (0x2B) zusammen mit den angegebenen Koordinaten.
 *
 * @param  X1 Startadresse der Spalte.
 * @param  Y1 Startadresse der Zeile.
 * @param  X2 Endadresse der Spalte.
 * @param  Y2 Endadresse der Zeile.
 *
 * @note   Diese Funktion wird häufig vor dem Schreiben von Pixeldaten verwendet, um den Zielbereich
 *         auf dem Display zu definieren.
 */
void ILI9341_SetAddress(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2) {
    // Erstellen der Parameter für den Spaltenadressbereich
    uint8_t params[4] = {X1 >> 8, X1, X2 >> 8, X2};
    // Erstellen der Parameter für den Zeilenadressbereich
    uint8_t params2[4] = {Y1 >> 8, Y1, Y2 >> 8, Y2};

    // Senden des Spaltenadressbefehls mit den entsprechenden Parametern
    ILI9341_SendCommandWithParam_8Bit(0x2A, params, 4);

    // Senden des Zeilenadressbefehls mit den entsprechenden Parametern
    ILI9341_SendCommandWithParam_8Bit(0x2B, params2, 4);
}



/**
 * @brief  Füllt den gesamten Bildschirm mit einer einzigen Farbe.
 *
 * Diese Funktion nutzt die ILI9341_SetAddress-Funktion, um das Adressfenster auf
 * die gesamte Bildschirmfläche zu setzen (von 0,0 bis zur maximalen Breite und Höhe),
 * und ruft dann ILI9341_DrawColourBurst auf, um alle Pixel mit der angegebenen Farbe zu füllen.
 *
 * @param  Colour Die 16-Bit-Farbe (RGB565-Format), mit der der Bildschirm gefüllt werden soll.
 *
 * @note   Diese Funktion ist eine effiziente Methode zum Löschen des gesamten Bildschirms,
 *         da sie alle Pixel in einem einzigen Burst aktualisiert.
 */
void ILI9341_FillScreen(uint16_t Colour) {
	ILI9341_SetAddress(0,0,ILI9341_WIDTH,ILI9341_HEIGHT);
	ILI9341_DrawColourBurst(Colour,ILI9341_WIDTH*ILI9341_HEIGHT);
}


/**
 * @brief  Zeichnet den Umriss eines Rechtecks auf dem ILI9341-Display.
 *
 * Diese Funktion zeichnet ein Rechteck mit den angegebenen Abmessungen und der angegebenen Farbe,
 * indem sie zwei horizontale und zwei vertikale Linien verbindet, um den Umriss zu bilden.
 * Es wird nur der Umriss (Rahmen) gezeichnet, nicht das gefüllte Innere.
 *
 * Der Algorithmus besteht aus vier Schritten:
 * 1. Zeichnen der oberen horizontalen Linie
 * 2. Zeichnen der unteren horizontalen Linie
 * 3. Zeichnen der linken vertikalen Linie
 * 4. Zeichnen der rechten vertikalen Linie
 *
 * @param x      Die x-Koordinate der oberen linken Ecke des Rechtecks.
 * @param y      Die y-Koordinate der oberen linken Ecke des Rechtecks.
 * @param width  Die Breite des Rechtecks in Pixeln.
 * @param height Die Höhe des Rechtecks in Pixeln.
 * @param color  Die Farbe des Rechteck-Umrisses im 16-Bit RGB565-Format.
 *
 * @note   Im Gegensatz zu ILI9341_DrawRectangle() oder ILI9341_fillRect(), die gefüllte
 *         Rechtecke zeichnen, erstellt diese Funktion nur den Rahmen des Rechtecks.
 */
void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    // Draw the top horizontal line
    ILI9341_DrawHLine(x, y, width, color);

    // Draw the bottom horizontal line
    ILI9341_DrawHLine(x, y + height - 1, width, color);

    // Draw the left vertical line
    ILI9341_DrawVLine(x, y, height, color);

    // Draw the right vertical line
    ILI9341_DrawVLine(x + width - 1, y, height, color);
}

/**
 * @brief  Sendet einen Burst einer bestimmten Farbe an das ILI9341-Display für eine angegebene Anzahl von Pixeln.
 *
 * Diese Funktion sendet Farbdaten als Burst an das Display unter Verwendung des Memory-Write-Befehls (0x2C).
 * Sie optimiert die Übertragung, indem sie die Daten in Blöcke mit einer maximalen Größe (BURST_MAX_SIZE)
 * aufteilt, um Pufferüberläufe zu vermeiden und die Leistung zu verbessern. Die Farbe wird im
 * 16-Bit-Format (RGB565) gesendet.
 *
 * Der Algorithmus funktioniert wie folgt:
 * 1. Senden des Memory-Write-Befehls an das Display
 * 2. Berechnung der optimalen Puffergröße basierend auf der Anzahl der zu sendenden Pixel
 * 3. Vorbereiten des Burst-Puffers mit der angegebenen Farbe
 * 4. Aufteilen der Gesamtdaten in vollständige Blöcke und einen möglichen Rest
 * 5. Senden der vollständigen Blöcke und des Rests über die SPI-Schnittstelle
 *
 * @param  Colour Die 16-Bit-Farbwert (RGB565), der gesendet werden soll.
 * @param  Size   Die Anzahl der Pixel, die mit der angegebenen Farbe gefüllt werden sollen.
 *
 * @note   Die Funktion teilt die Daten in Blöcke der Größe BURST_MAX_SIZE auf, um eine effiziente
 *         Übertragung zu gewährleisten. Wenn die Gesamtgröße BURST_MAX_SIZE überschreitet, werden
 *         die Daten in Blöcken gesendet und der Rest separat behandelt.
 */
void ILI9341_DrawColourBurst(uint16_t Colour, uint32_t Size) {

	//COMMAND Memory Write
	ILI9341_SendCommand(0x2C);

	// Calculate the buffer size for transmission
	uint32_t Buffer_Size = 0;
	if((Size*2) < BURST_MAX_SIZE)
	{
		Buffer_Size = Size; // Use the full size if it fits within the buffer limit
	} else {
		Buffer_Size = BURST_MAX_SIZE; // Otherwise, use the maximum buffer size
	}

	// Exit if the buffer size is zero
	if (Buffer_Size == 0) return;

	ILI9341_SetData();
	ILI9341_ChipSelect();

	// Prepare the burst buffer with the specified color
	unsigned char chifted = 	Colour>>8; // Extract the high byte of the color
	unsigned char burst_buffer[Buffer_Size];
	for(uint32_t j = 0; j < Buffer_Size; j+=2)
		{
			burst_buffer[j] = 	chifted; // High byte of the color
			burst_buffer[j+1] = Colour; // Low byte of the color
		}

	// Calculate the total data size and how to split it into blocks
	uint32_t Sending_Size = Size*2; // Total bytes to send (2 bytes per pixel)
	uint32_t Sending_in_Block = Sending_Size/Buffer_Size; // Number of full blocks
	uint32_t Remainder_from_block = Sending_Size%Buffer_Size; // Remaining bytes

	// Send full blocks of data
	if(Sending_in_Block != 0)
	{
		for(uint32_t j = 0; j < (Sending_in_Block); j++)
			{
			HAL_SPI_Transmit(ILI9341_SPI, (unsigned char *)burst_buffer, Buffer_Size, 100);
			}
	}

	// Send the remaining bytes
	HAL_SPI_Transmit(ILI9341_SPI, (unsigned char *)burst_buffer, Remainder_from_block, 10);
	ILI9341_ChipDeselect();
}

/**
 * Sets the rotation of the ILI9341 display.
 *
 * This function configures the display's orientation by sending the Memory Access Control (0x36) command
 * followed by the appropriate parameter to set the rotation. It also updates the display's width and height
 * based on the selected rotation.
 *
 * @param Rotation The desired rotation mode. Supported modes are:
 *                 - SCREEN_VERTICAL_1: Vertical orientation (default).
 *                 - SCREEN_HORIZONTAL_1: Horizontal orientation (90 degrees).
 *                 - SCREEN_VERTICAL_2: Vertical orientation flipped (180 degrees).
 *                 - SCREEN_HORIZONTAL_2: Horizontal orientation flipped (270 degrees).
 *
 * @note If an invalid rotation mode is provided, the function exits without making any changes.
 */
void ILI9341_SetRotation(uint8_t Rotation) {
	ILI9341_SendCommand(0x36);
	HAL_Delay(1);
	uint8_t data;
	switch(Rotation)
		{
			case SCREEN_VERTICAL_1:
				data = 0x40|0x08;
				ILI9341_WIDTH = 240;
				ILI9341_HEIGHT = 320;
				break;
			case SCREEN_HORIZONTAL_1:
				data = (0x20|0x08);
				ILI9341_WIDTH  = 320;
				ILI9341_HEIGHT = 240;
				break;
			case SCREEN_VERTICAL_2:
				data = (0x80|0x08);
				ILI9341_WIDTH  = 240;
				ILI9341_HEIGHT = 320;
				break;
			case SCREEN_HORIZONTAL_2:
				data = (0x40|0x80|0x20|0x08);
				ILI9341_WIDTH  = 320;
				ILI9341_HEIGHT = 240;
				break;
			default:
				//EXIT IF SCREEN ROTATION NOT VALID!
				return;
		}
	ILI9341_SendData(&data, 1);

}

/**
 * @brief  Legt die Ausrichtung des ILI9341-Displays fest.
 *
 * Diese Funktion konfiguriert die Ausrichtung des Displays durch Senden des
 * Memory Access Control-Befehls (0x36) gefolgt vom entsprechenden MADCTL-Parameter.
 * Sie aktualisiert auch die Breite und Höhe des Displays basierend auf der gewählten
 * Ausrichtung.
 *
 * Die MADCTL-Bits haben folgende Bedeutung:
 *   - MY: Row Address Order (0=Top to Bottom, 1=Bottom to Top)
 *   - MX: Column Address Order (0=Left to Right, 1=Right to Left)
 *   - MV: Row/Column Exchange (0=Normal, 1=Vertauscht)
 *   - ML: Vertical Refresh Order (0=Normal, 1=Umgekehrt)
 *   - BGR: RGB/BGR Order (0=RGB, 1=BGR Format)
 *
 * @param orientation Der gewünschte Ausrichtungsmodus. Unterstützte Modi sind:
 *                    - ILI9341_PORTRAIT: 0° Portrait.
 *                    - ILI9341_LANDSCAPE: 90° Querformat.
 *                    - ILI9341_PORTRAIT_INVERTED: 180° Invertiertes Portrait.
 *                    - ILI9341_LANDSCAPE_INVERTED: 270° Invertiertes Querformat.
 *                    - TEST: Benutzerdefinierte Ausrichtung für Testzwecke.
 *                    - ILI9341_PORTRAIT_TRUE: Echte Portrait-Ausrichtung.
 *
 * @note Diese Funktion aktualisiert die globalen Variablen ILI9341_WIDTH und
 *       ILI9341_HEIGHT entsprechend der gewählten Ausrichtung (240x320 oder 320x240).
 */
void ILI9341_SetOrientation(ILI9341_Orientation orientation) {
    uint8_t madctl = 0;

    switch(orientation) {
        case ILI9341_PORTRAIT: // 0° Portrait
            madctl = 0x08;  // MY=0, MX=0, MV=0, ML=0, BGR=1
            ILI9341_WIDTH = 240;
            ILI9341_HEIGHT = 320;
            break;

        case ILI9341_LANDSCAPE: // 90° Landscape
            madctl = 0x68;  // MY=0, MX=1, MV=1, ML=0, BGR=1
            ILI9341_WIDTH = 320;
            ILI9341_HEIGHT = 240;
            break;

        case ILI9341_PORTRAIT_INVERTED: // 180° Inverted Portrait
            madctl = 0xC8;  // MY=1, MX=1, MV=0, ML=0, BGR=1
            ILI9341_WIDTH = 240;
            ILI9341_HEIGHT = 320;
            break;

        case ILI9341_LANDSCAPE_INVERTED: // 270° Inverted Landscape
            madctl = 0xA8;  // MY=1, MX=0, MV=1, ML=0, BGR=1
            ILI9341_WIDTH = 320;
            ILI9341_HEIGHT = 240;
            break;
        case TEST:
        	madctl = 0b00101000;  // MY=1, MX=0, MV=1, ML=0, BGR=1
			ILI9341_WIDTH = 320;
			ILI9341_HEIGHT = 240;
			break;
        case ILI9341_PORTRAIT_TRUE:
        	madctl = 0b10001000;  // MY=0, MX=0, MV=0, ML=0, BGR=1
			ILI9341_WIDTH = 240;
			ILI9341_HEIGHT = 320;
    }

    // Send command to set MADCTL
    ILI9341_SendCommandWithParam_8Bit(0x36, &madctl, 1);
}

/**
 * @brief  Zeichnet ein Rechteck auf dem ILI9341-Display.
 * @param  x: X-Koordinate der oberen linken Ecke des Rechtecks.
 * @param  y: Y-Koordinate der oberen linken Ecke des Rechtecks.
 * @param  w: Breite des Rechtecks.
 * @param  h: Höhe des Rechtecks.
 * @param  color: Farbe des Rechtecks (16-Bit RGB565 Format).
 * @retval None
 */
void ILI9341_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	ILI9341_SetAddress(x, y, x+w-1, y+h-1);
	ILI9341_DrawColourBurst(color,w*h);
}

/**
 * @brief  Füllt einen rechteckigen Bereich auf dem ILI9341-Display mit einer bestimmten Farbe.
 *
 * Diese Funktion setzt das Adressfenster auf den angegebenen rechteckigen Bereich und
 * füllt diesen mit der angegebenen Farbe, indem sie einen Farb-Burst verwendet. Der rechteckige
 * Bereich wird durch seine obere linke Ecke, Breite und Höhe definiert.
 *
 * Die Funktion nutzt folgende Hilfsfunktionen:
 * - ILI9341_SetAddress(): Legt den zu bearbeitenden Adressbereich auf dem Display fest
 * - ILI9341_DrawColourBurst(): Füllt den festgelegten Bereich mit der angegebenen Farbe
 *
 * @param x     Die x-Koordinate der oberen linken Ecke des Rechtecks.
 * @param y     Die y-Koordinate der oberen linken Ecke des Rechtecks.
 * @param w     Die Breite des Rechtecks in Pixeln.
 * @param h     Die Höhe des Rechtecks in Pixeln.
 * @param color Der 16-Bit-Farbwert (RGB565-Format) zum Füllen des Rechtecks.
 *
 * @note   Diese Funktion ist ähnlich zu ILI9341_DrawRectangle(), verwendet jedoch
 *         andere Parametertypen (int16_t statt uint16_t) für die Positionierung.
 */
void ILI9341_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
	ILI9341_SetAddress(x, y, x+w-1, y+h-1);
	ILI9341_DrawColourBurst(color,w*h);
}

/**
 * @brief  Zeichnet eine horizontale Linie auf dem ILI9341-Display.
 *
 * Diese Funktion zeichnet eine horizontale Linie von einem bestimmten Startpunkt (x,y)
 * mit einer angegebenen Breite und Farbe. Der Algorithmus nutzt optimierte
 * Zeichenmethoden, indem er:
 * 1. Das Adressfenster auf die exakte Größe der Linie setzt (von (x,y) bis (x+w-1,y))
 * 2. Einen Farb-Burst verwendet, um alle Pixel der Linie in einem einzigen Vorgang zu füllen
 *
 * @param  x     Die x-Koordinate des Startpunkts der Linie.
 * @param  y     Die y-Koordinate des Startpunkts der Linie.
 * @param  w     Die Breite (Länge) der Linie in Pixeln.
 * @param  color Die Farbe der Linie im 16-Bit RGB565-Format.
 *
 * @note   Diese Funktion ist effizienter als das separate Zeichnen einzelner Pixel,
 *         da sie die schnelle Burst-Methode verwendet.
 */
void ILI9341_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
	ILI9341_SetAddress(x, y, x+w-1,y);
	ILI9341_DrawColourBurst(color,w);
}
/**
 * @brief  Zeichnet eine vertikale Linie auf dem ILI9341-Display.
 *
 * Diese Funktion zeichnet eine vertikale Linie von einem bestimmten Startpunkt (x,y)
 * mit einer angegebenen Höhe und Farbe. Der Algorithmus nutzt optimierte
 * Zeichenmethoden, indem er:
 * 1. Das Adressfenster auf die exakte Größe der Linie setzt (von (x,y) bis (x,y+h-1))
 * 2. Einen Farb-Burst verwendet, um alle Pixel der Linie in einem einzigen Vorgang zu füllen
 *
 * @param  x     Die x-Koordinate des Startpunkts der Linie.
 * @param  y     Die y-Koordinate des Startpunkts der Linie.
 * @param  h     Die Höhe (Länge) der Linie in Pixeln.
 * @param  color Die Farbe der Linie im 16-Bit RGB565-Format.
 *
 * @note   Diese Funktion ist effizienter als das separate Zeichnen einzelner Pixel,
 *         da sie die schnelle Burst-Methode verwendet.
 */
void ILI9341_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
	ILI9341_SetAddress(x, y, x,y+h-1);
	ILI9341_DrawColourBurst(color,h);
}

/**
 * @brief  Zeichnet einen einzelnen Pixel auf dem ILI9341-Display.
 *
 * Diese Funktion setzt einen einzelnen Pixel an den angegebenen Koordinaten mit der
 * gewünschten Farbe. Der Algorithmus funktioniert wie folgt:
 * 1. Setzen des Adressfensters auf genau einen Pixel an Position (x,y)
 * 2. Aufteilen des 16-Bit-Farbwerts in zwei 8-Bit-Werte (High-Byte und Low-Byte)
 * 3. Senden des Memory-Write-Befehls (0x2C) an das Display
 * 4. Übertragen der Farbdaten über die SPI-Schnittstelle
 *
 * @param  x     Die x-Koordinate des zu zeichnenden Pixels.
 * @param  y     Die y-Koordinate des zu zeichnenden Pixels.
 * @param  color Die Farbe des Pixels im 16-Bit RGB565-Format.
 *
 * @note   Diese Funktion ist der grundlegende Baustein für alle komplexeren Zeichenoperationen,
 *         wird aber für größere Zeichenoperationen zugunsten effizienterer Methoden wie
 *         ILI9341_DrawColourBurst() vermieden.
 */
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	ILI9341_SetAddress(x, y, x,y);
	unsigned char cholor = color>>8;
	unsigned char buffer[2] = {cholor,color};
	//COMMAND Memory Write
	ILI9341_SendCommand(0x2C);

	ILI9341_SendData(buffer, 2);
}
/**
 * @brief  Zeichnet einen einzelnen Pixel auf dem ILI9341-Display mit RGB-Farbkomponenten.
 *
 * Diese Funktion ermöglicht das Setzen eines Pixels mit separaten 8-Bit RGB-Farbwerten,
 * was eine intuitivere Farbdefinition im Vergleich zum direkten RGB565-Format ermöglicht.
 * Der Algorithmus führt folgende Schritte aus:
 * 1. Setzt das Adressfenster auf genau einen Pixel an Position (x,y)
 * 2. Konvertiert die 8-Bit RGB-Werte (R: 0-255, G: 0-255, B: 0-255) in das 16-Bit RGB565-Format:
 *    - Rot: Die oberen 5 Bits werden verwendet (R & 0xF8) und um 8 Bits nach links verschoben
 *    - Grün: Die oberen 6 Bits werden verwendet (G & 0xFC) und um 3 Bits nach links verschoben
 *    - Blau: Die oberen 5 Bits werden verwendet (B >> 3)
 * 3. Ruft ILI9341_DrawPixel() auf, um den Pixel mit der berechneten Farbe zu setzen
 *
 * @param  x  Die x-Koordinate des zu zeichnenden Pixels.
 * @param  y  Die y-Koordinate des zu zeichnenden Pixels.
 * @param  r  Die Rot-Komponente der Farbe (8-Bit, 0-255).
 * @param  g  Die Grün-Komponente der Farbe (8-Bit, 0-255).
 * @param  b  Die Blau-Komponente der Farbe (8-Bit, 0-255).
 *
 */
void ILI9341_DrawPixelRGB(uint16_t x,uint16_t y,uint8_t r, uint8_t g, uint8_t b){
	ILI9341_SetAddress(x, y, x, y);

	// Convert 8-bit RGB values to 16-bit RGB565 format
	uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

	// Set the pixel at (x, y) with the calculated color
	ILI9341_DrawPixel(x, y, color);
}


/**
 * @brief  Draws the outline of a circle on the display.
 * @param  x_pos: X-coordinate of the circle center.
 * @param  y_pos: Y-coordinate of the circle center.
 * @param  r: Radius of the circle.
 * @param  color: Color of the circle outline (16-bit RGB565 format).
 * @retval None
 *
 * This function implements the Midpoint Circle Algorithm to efficiently
 * draw a circle by plotting symmetric points in all eight octants.
 */
void ILI9341_DrawCircleOutline(uint16_t x_pos, uint16_t y_pos, uint8_t r, uint16_t color) {


	int x = r;
	int y = 0;
	int decisionOver2 = 1 - x; // Decision criterion divided by 2 evaluated at x=r, y=0

	while (y <= x) {
		// Octants are drawn in each step since the circle is symmetric
		ILI9341_DrawPixel(x_pos + x, y_pos + y,color); // Octant 1
		ILI9341_DrawPixel(x_pos + y, y_pos + x,color); // Octant 2
		ILI9341_DrawPixel(x_pos - y, y_pos + x,color); // Octant 3
		ILI9341_DrawPixel(x_pos - x, y_pos + y,color); // Octant 4
		ILI9341_DrawPixel(x_pos - x, y_pos - y,color); // Octant 5
		ILI9341_DrawPixel(x_pos - y, y_pos - x,color); // Octant 6
		ILI9341_DrawPixel(x_pos + y, y_pos - x,color); // Octant 7
		ILI9341_DrawPixel(x_pos + x, y_pos - y,color); // Octant 8

		y++;
		if (decisionOver2 <= 0) {
			decisionOver2 += 2 * y + 1; // Change decision criterion for y -> y+1
		} else {
			x--;
			decisionOver2 += 2 * (y - x) + 1; // Change decision criterion for y -> y+1, x -> x-1
		}
	}


}

void ILI9341_DrawCircle(uint16_t x_pos, uint16_t y_pos, uint8_t r, uint16_t color) {
	int x = r;
	int y = 0;
	int decisionOver2 = 1 - x; // Decision criterion divided by 2 evaluated at x=r, y=0

	while (y <= x) {
		// Draw horizontal lines between points in each octant
		ILI9341_DrawHLine(x_pos - x, y_pos + y, 2 * x + 1,color); // Top segment
		ILI9341_DrawHLine(x_pos - y, y_pos + x, 2 * y + 1,color); // Right segment
		ILI9341_DrawHLine(x_pos - x, y_pos - y, 2 * x + 1,color); // Bottom segment
		ILI9341_DrawHLine(x_pos - y, y_pos - x, 2 * y + 1,color); // Left segment

		y++;
		if (decisionOver2 <= 0) {
			decisionOver2 += 2 * y + 1; // Change decision criterion for y -> y+1
		} else {
			x--;
			decisionOver2 += 2 * (y - x) + 1; // Change decision criterion for y -> y+1, x -> x-1
		}
	}
}

void ILI9341_DrawFilledCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        // Draw horizontal lines to fill the circle
        ILI9341_DrawHLine(x0 - x, y0 + y, 2 * x, color);
        ILI9341_DrawHLine(x0 - x, y0 - y, 2 * x, color);
        ILI9341_DrawHLine(x0 - y, y0 + x, 2 * y, color);
        ILI9341_DrawHLine(x0 - y, y0 - x, 2 * y, color);

        // Update the midpoint error
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void ILI9341_SecretCommand() {
	uint8_t param = 0x10;
	ILI9341_SendCommandWithParam_8Bit(0xD9, &param, 1);
}

void DisplayImageArray(const uint16_t* imageData, uint16_t width, uint16_t height)
{

  /* Calculate screen centering (if needed) */
  uint16_t x_start = (320 - width) / 2;  // Assuming 320x240 display
  uint16_t y_start = (240 - height) / 2;

  /* Set display address window */
  ILI9341_SetAddress(x_start, y_start, x_start + width - 1, y_start + height - 1);

  ILI9341_SendCommand(0x2C);

  /* Send image data to display */
  ILI9341_SetData();
  ILI9341_ChipSelect();

  /* Send data in chunks to work with DMA buffer size limitations */
  #define CHUNK_SIZE 1024  // Adjust based on your available memory

  for (uint32_t i = 0; i < width * height; i += CHUNK_SIZE) {
    uint32_t chunk = ((width * height) - i > CHUNK_SIZE) ? CHUNK_SIZE : (width * height) - i;

    /* Use DMA for faster transfer */
    HAL_SPI_Transmit(ILI9341_SPI, (uint8_t*)&imageData[i], chunk * 2,HAL_MAX_DELAY);

    /* Wait for DMA to complete */
    while (HAL_SPI_GetState(ILI9341_SPI) != HAL_SPI_STATE_READY) {}
  }

  ILI9341_ChipDeselect();
}

/**
 * @brief  Zeichnet ein Bild auf dem ILI9341-Display.
 *
 * Diese Funktion zeichnet ein Bild an der angegebenen Position (x,y) mit der
 * angegebenen Größe (width x height). Das Bild wird aus einem Byte-Array gelesen,
 * das die Pixel im RGB565-Format enthält (2 Bytes pro Pixel). Der Algorithmus
 * führt folgende Schritte aus:
 * 1. Setzt das Adressfenster auf den Bereich, in dem das Bild gezeichnet werden soll
 * 2. Sendet den Memory-Write-Befehl (0x2C) an das Display
 * 3. Überträgt die Bilddaten in einem Durchgang an das Display
 *
 * @param  x      Die x-Koordinate der oberen linken Ecke des Bildes.
 * @param  y      Die y-Koordinate der oberen linken Ecke des Bildes.
 * @param  width  Die Breite des Bildes in Pixeln.
 * @param  height Die Höhe des Bildes in Pixeln.
 * @param  image  Zeiger auf das Byte-Array, das die Bilddaten im RGB565-Format enthält
 *                (2 Bytes pro Pixel, insgesamt width*height*2 Bytes).
 *
 * @note   Die Bilddaten müssen im RGB565-Format vorliegen (High-Byte zuerst),
 *         wobei jedes Pixel durch 2 Bytes dargestellt wird. Das Array muss
 *         mindestens width*height*2 Bytes groß sein.
 */
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image){
    // Set the drawing window
    ILI9341_SetAddress(x, y, x + width - 1, y + height - 1);

    ILI9341_SendCommand(0x2C);


	ILI9341_SendData(image, width*height*2);

}

void ILI9341_DrawChar(char Character, uint16_t X, uint16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
	uint8_t 	function_char;
    uint16_t 	i,j;

	function_char = Character;

    if (function_char < ' ') {
        Character = 0;
    } else {
    	function_char -= 32;
	}

	char temp[CHAR_WIDTH];
	for(uint16_t k = 0; k<CHAR_WIDTH; k++)
	{
	temp[k] = stdfont[function_char][k];
	}

    // Draw pixels
	//ILI9341_DrawRectangle(X, Y, CHAR_WIDTH*Size, CHAR_HEIGHT*Size, Background_Colour);
    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (temp[j] & (1<<i)) {
				if(Size == 1)
				{
					ILI9341_DrawPixel(X+j, Y+i, Colour);
				}
				else
				{
					ILI9341_DrawRectangle(X+(j*Size), Y+(i*Size), Size, Size, Colour);
				}
            }
        }
    }
}

void ILI9341_DrawText(const char* Text, uint16_t X, uint16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
    while (*Text) {
        // Check if the next character will fit within the display's width
        if (X + CHAR_WIDTH * Size > ILI9341_WIDTH) {
            break; // Stop drawing if the next character exceeds the display width
        }

    	ILI9341_DrawChar(*Text++, X, Y, Colour, Size, Background_Colour);
        X += CHAR_WIDTH*Size;
    }
}


void ILI9341_DrawBorder(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t borderSize, uint16_t color) {
	// Draw top border (above the rectangle)
	ILI9341_DrawRectangle(x - borderSize, y - borderSize, width + 2 * borderSize, borderSize, color);

	// Draw bottom border (below the rectangle)
	ILI9341_DrawRectangle(x - borderSize, y + height, width + 2 * borderSize, borderSize, color);

	// Draw left border (to the left of the rectangle)
	ILI9341_DrawRectangle(x - borderSize, y, borderSize, height, color);

	// Draw right border (to the right of the rectangle)
	ILI9341_DrawRectangle(x + width, y, borderSize, height, color);
}

// Function to draw a filled rounded rectangle
void ILI9341_DrawFilledRoundedRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
    // Draw the filled rectangle (excluding the rounded corners)
    ILI9341_DrawRectangle(x + radius, y, width - 2 * radius, height, color); // Middle rectangle
    ILI9341_DrawRectangle(x, y + radius, width, height - 2 * radius, color); // Middle rectangle

    // Draw the four rounded corners
    ILI9341_DrawFilledCircle(x + radius, y + radius, radius, color); // Top-left corner
    ILI9341_DrawFilledCircle(x + width - radius - 1, y + radius, radius, color); // Top-right corner
    ILI9341_DrawFilledCircle(x + radius, y + height - radius - 1, radius, color); // Bottom-left corner
    ILI9341_DrawFilledCircle(x + width - radius - 1, y + height - radius - 1, radius, color); // Bottom-right corner
}

void ILI9341_DrawRoundedRectWithBorder(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t fillColor, uint16_t borderColor, uint16_t borderSize) {

    // Draw the outer rounded rectangle (border)
    ILI9341_DrawFilledRoundedRect(x, y, width, height, radius, borderColor);

    // Draw the inner rounded rectangle (fill)
    ILI9341_DrawFilledRoundedRect(x + borderSize, y + borderSize, width - 2 * borderSize, height - 2 * borderSize, radius, fillColor);

}


void setTextColor(uint16_t c){
	textcolor = textbgcolor = c;
}

void ILI9341_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color){
    // Draw the central rectangle
    ILI9341_fillRect(x + r, y, w - 2 * r, h, color);

    // Draw the four rounded corners
    ILI9341_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color); // Top-right corner
    ILI9341_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);         // Top-left corner
    ILI9341_FillCircleHelper(x + r, y + h - r - 1, r, 4, h - 2 * r - 1, color); // Bottom-left corner
    ILI9341_FillCircleHelper(x + w - r - 1, y + h - r - 1, r, 8, h - 2 * r - 1, color); // Bottom-right corner
}
void ILI9341_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color){
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1) {
            ILI9341_DrawVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            ILI9341_DrawVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            ILI9341_DrawVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            ILI9341_DrawVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

// Macro to swap two values
#define SWAP(a, b) { int16_t t = a; a = b; b = t; }



/**
 * @brief  Zeichnet ein Bild von einer Binärdatei auf der SD-Karte auf dem ILI9341-Display.
 *
 * Diese Funktion lädt und zeigt eine binäre Bilddatei von der SD-Karte auf dem Display an.
 * Der Algorithmus funktioniert wie folgt:
 * 1. Mounten der SD-Karte
 * 2. Öffnen der angegebenen Datei im Lesemodus
 * 3. Zeilenweises Lesen der Bilddaten (RGB565-Format, 2 Bytes pro Pixel)
 * 4. Zeichnen jeder Zeile auf dem Display mit ILI9341_DrawImage()
 * 5. Schließen der Datei und Unmounten der SD-Karte nach Abschluss
 *
 * @param  filename Der Pfad zur Binärdatei auf der SD-Karte.
 * @param  x        Die x-Koordinate der oberen linken Ecke des Bildes auf dem Display.
 * @param  y        Die y-Koordinate der oberen linken Ecke des Bildes auf dem Display.
 * @param  width    Die Breite des Bildes in Pixeln.
 * @param  height   Die Höhe des Bildes in Pixeln.
 *
 * @note   Die Binärdatei muss Pixeldaten im RGB565-Format enthalten (2 Bytes pro Pixel).
 *         Die Funktion liest das Bild zeilenweise ein, um Speicher zu sparen, und
 *         verwendet einen Puffer der Größe width*2 Bytes für jede Zeile.
 *         Eine Überprüfung der Dateigröße ist auskommentiert, kann aber bei Bedarf
 *         aktiviert werden, um sicherzustellen, dass die Datei genügend Daten enthält.
 */
void ILI9341_DrawBinaryFile(const char* filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height){

	if (!mountSD()){
		printf("Could not mount SDCard");
		return;
	}

	FIL file = {0};          // File object
	UINT bytesRead;    // Number of bytes read
	uint8_t buffer[width*2]; // Buffer for reading pixel data (adjust size as needed)

	// Open the file
	FRESULT res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		printf("Failed to open file: %d\n", res);
		return;
	}

	// Read and display the binary file
	for (uint16_t row = 0; row < height; row++) {
		// Read a row of pixel data
		res = f_read(&file, buffer, width * 2, &bytesRead);


		if (res != FR_OK || bytesRead != width * sizeof(uint16_t)) {
			printf("Failed to read file: %d\n", res);
			f_close(&file);
			return;
		}

		// Draw the row on the screen
		ILI9341_DrawImage(x, y+row, width, 1, buffer);
	}

	// Close the file
	f_close(&file);

	unmountSD();
}
