

#include "SSD1306.h"

#include <stdlib.h>
#include <string.h>


/*
 * DATASHEET: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
 */


void ssd1306_Reset(void) {

}

/**
 * @brief Sendet einen Befehl an das SSD1306-Display.
 *
 * Diese Funktion schreibt ein einzelnes Byte als Befehl an das SSD1306-Display
 * über die I2C-Schnittstelle. Der Befehl wird im Kontrollbyte als Befehl (0x00) markiert.
 *
 * @param byte Das zu sendende Befehlsbyte.
 */
void ssd1306_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}


/**
 * @brief Sendet Daten an das SSD1306-Display.
 *
 * Diese Funktion schreibt einen Datenpuffer an das SSD1306-Display
 * über die I2C-Schnittstelle. Der Kontrollbyte wird als Daten (0x40) markiert,
 * um anzuzeigen, dass die nachfolgenden Bytes als Anzeigedaten interpretiert werden.
 *
 * @param buffer Zeiger auf den Datenpuffer, der gesendet werden soll.
 * @param buff_size Größe des zu sendenden Datenpuffers in Bytes.
 */
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;


/**
 * @brief Initialisiert das SSD1306-OLED-Display.
 *
 * Diese Funktion führt die Initialisierung des SSD1306-Displays durch, indem sie
 * die notwendigen Befehle sendet, um das Display in den Betriebszustand zu versetzen.
 * Dazu gehören das Zurücksetzen des Displays, das Konfigurieren von Anzeigeparametern
 * wie Adressierungsmodus, Kontrast, Multiplex-Verhältnis und andere Hardwareeinstellungen.
 * Nach der Initialisierung wird der Bildschirm gelöscht und der Pufferinhalt auf das Display geschrieben.
 *
 * @note Diese Funktion muss vor der Nutzung anderer Display-Funktionen aufgerufen werden.
 */
void ssd1306_Init(void) {
    ssd1306_Reset();

    HAL_Delay(100);

    // Init OLED
    ssd1306_SetDisplayOn(0); // Display ausschalten

    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=34
    ssd1306_WriteCommand(0x20); // Setze Adressierungsmodus

    /**
         * ADRESSIERUNGSMODI DES SSD1306:
         *
         * Der SSD1306 unterstützt drei verschiedene Adressierungsmodi, die bestimmen, wie der
         * interne Speicherzeiger nach jedem Datenzugriff aktualisiert wird:
         *
         * 1. Horizontaler Adressierungsmodus (0x00):
         *    - Zeiger bewegt sich horizontal von links nach rechts innerhalb einer Seite
         *    - Nach Erreichen des rechten Seitenrands springt er zur ersten Spalte der nächsten Seite
         *    - Nach der letzten Seite beginnt er wieder bei der ersten Spalte der ersten Seite
         *    - Ideal zum sequentiellen Beschreiben des gesamten Bildschirms
         *
         * +--------+--------+--------+--------+--------+
         * |        | COL 0  | COL 1  |  ...   | COL127 |
         * +--------+--------+--------+--------+--------+
         * | PAGE0 |   -->  |   -->  |  ...   |  -->   | --> PAGE1
         * | PAGE1 |   -->  |   -->  |  ...   |  -->   | --> PAGE2
         * |  ...  |   ...  |   ...  |  ...   |  ...   | --> PAGEN+1
         * | PAGE6 |   -->  |   -->  |  ...   |  -->   | --> PAGE7
         * | PAGE7 |   -->  |   -->  |  ...   |  -->   | --> PAGE0
         * +--------+--------+--------+--------+--------+
         *
         * 2. Vertikaler Adressierungsmodus (0x01):
         *    - Zeiger bewegt sich vertikal von oben nach unten innerhalb einer Spalte
         *    - Nach Erreichen der unteren Spaltengrenze springt er zur nächsten Spalte und beginnt oben
         *    - Nach der letzten Spalte beginnt er wieder bei der ersten Spalte ganz oben
         *    - Nützlich für bestimmte vertikale Anzeigemuster
         *
         * +--------+--------+--------+--------+--------+
         * |        | COL 0  | COL 1  |  ...   | COL127 |
         * +--------+--------+--------+--------+--------+
         * | PAGE0 |   ↓     |   ↓     |  ...   |   ↓    |
         * | PAGE1 |   ↓     |   ↓     |  ...   |   ↓    |
         * |  ...  |  ...    |  ...    |  ...   |  ...   |
         * | PAGE6 |   ↓     |   ↓     |  ...   |   ↓    |
         * | PAGE7 |   ↓     |   ↓     |  ...   |   ↓    |
         * +--------+--------+--------+--------+--------+
         *            ↓       ↓              ↓      ↓
         *            COL1   COL2           COLN+1  COL0
         *
         * 3. Seiten-Adressierungsmodus (0x02):
         *    - Der aktive Bereich ist auf eine Seite beschränkt
         *    - Der Zeiger bewegt sich nur horizontal innerhalb der ausgewählten Seite
         *    - Nach Erreichen des Seitenendes springt er zurück zum Seitenanfang
         *    - Um zu einer anderen Seite zu wechseln, muss explizit ein Befehl gesendet werden
         *    - In ssd1306_UpdateScreen() wird jede Seite mit 0xB0 + i adressiert
         *
         * +--------+--------+--------+--------+--------+
         * |        | COL 0  | COL 1  |  ...   | COL127 |
         * +--------+--------+--------+--------+--------+
         * | PAGE0 |   -->  |   -->  |  ...   |  -->   |
         * | PAGE1 |   -->  |   -->  |  ...   |  -->   |
         * |  ...  |   ...  |   ...  |  ...   |  ...   |
         * | PAGE6 |   -->  |   -->  |  ...   |  -->   |
         * | PAGE7 |   -->  |   -->  |  ...   |  -->   |
         * +--------+--------+--------+--------+--------+
         *
         */
    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=35
    ssd1306_WriteCommand(0x00); // Horizontal Addressing Mode

    /**
     * Setzt die Startadresse für den Page Addressing Mode.
     *
     * Im Page Addressing Mode wird der Speicher in Seiten unterteilt,
     * und diese Funktion legt die Startadresse für die aktuelle Seite fest.
     * Jede Seite repräsentiert 8 vertikale Pixel, und die Adresse wird
     * durch Hinzufügen von 0xB0 zu der gewünschten Seitenzahl bestimmt. (0xB0 bis 0xB7)
     *
     * Diese Funktion wird typischerweise verwendet, um den Speicherbereich
     * für das Schreiben von Daten im Page Addressing Mode zu definieren.
     */

    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=31
    ssd1306_WriteCommand(0xB0); // Setze Startadresse für Page Addressing Mode


    ssd1306_WriteCommand(0xC8); // Setze COM-Ausgabescanausrichtung (umgekehrter Modus)



    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=30
    ssd1306_WriteCommand(0x00); // Setze niedrige Spaltenadresse



    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=30
    ssd1306_WriteCommand(0x10); // Setze hohe Spaltenadresse


    /**
    * @brief Display-Startzeilenadresse festlegen.
    *
    * Der Befehl 0x40 definiert die RAM-Startzeilenadresse (0 bis 63) für die Display-Anzeige.
    * Er bestimmt, welche Zeile im RAM als erste Zeile auf dem Display angezeigt wird, was für
    * hardwarebasierte vertikale Bildlaufeffekte genutzt werden kann, ohne Pixel im Speicher zu verschieben.
    */
    ssd1306_WriteCommand(0x40); // Setze Startzeilenadresse

    /**
     * @brief Display-Kontrast einstellen.
     *
     * Stellt die Leuchtstärke der Pixel durch Anpassung der Spannung ein.
     * 0xFF ist der höchste Kontrastwert für maximale Helligkeit.
     * Niedrigere Werte reduzieren den Kontrast und sparen Strom.
     */
    ssd1306_SetContrast(0xFF); // Setze maximalen Kontrast

    /**
     * @brief Segment-Remap einstellen.
     *
     * 0xA0: Standard-Mapping (SEG0 = Spalte 0)
     * 0xA1: Umgekehrtes Mapping (SEG0 = Spalte 127)
     *
     * Dieser Befehl spiegelt das Display horizontal und wird verwendet,
     * um die korrekte Anzeige je nach Montagerichtung des Displays zu gewährleisten.
     */
    ssd1306_WriteCommand(0xA1); // Setze Segment-Remap

    /**
     * @brief Display-Modus einstellen.
     *
     * 0xA6: Normaler Modus (1 = helles Pixel, 0 = dunkles Pixel)
     * 0xA7: Invertierter Modus (0 = helles Pixel, 1 = dunkles Pixel)
     *
     * Bestimmt die Zuordnung zwischen RAM-Werten und Pixelzustand.
     */
    ssd1306_WriteCommand(0xA6); // Normale Farben


    //ssd1306_WriteCommand(0xA8); // Multiplex-Verhältnis


    /**
    * @brief Multiplex-Verhältnis einstellen.
    *
    * 0x3F entspricht dem Wert 64-1, aktiviert alle 64 Zeilen für ein 128x64 Display.
    * Dieser Parameter definiert, wie viele COM-Leitungen (Zeilen) das Display verwendet.
    * Für ein 128x32 Display wäre 0x1F (32-1) zu verwenden.
    */
    ssd1306_WriteCommand(0x3F);

    /**
     * @brief Display-Ausgabemodus festlegen.
     *
     * 0xA4: RAM-Inhalt anzeigen (normale Betriebsart)
     * 0xA5: RAM-Inhalt ignorieren und alle Pixel einschalten
     *
     * Im RAM-Modus werden die im Speicher abgelegten Bilddaten auf dem Display angezeigt.
     */
    ssd1306_WriteCommand(0xA4); // Ausgabe folgt RAM-Inhalt

    /**
     * @brief Vertikalen Display-Offset einstellen.
     *
     * 0xD3 ist der Befehlscode, gefolgt vom Offset-Wert (hier 0x00).
     * Der Offset verschiebt die Anzeige vertikal um 0-63 Zeilen.
     * Bei 0x00 beginnt die Anzeige bei COM0 ohne Verschiebung.
     */
    ssd1306_WriteCommand(0xD3); // Setze Display-Offset
    ssd1306_WriteCommand(0x00); // Kein Offset


    /**
    * @brief Taktfrequenz und Oszillator-Teiler einstellen.
    *
    * 0xD5 ist der Befehlscode, gefolgt vom Konfigurationswert.
    * Bei 0xF0: Obere 4 Bits (F) setzen die Oszillatorfrequenz auf Maximum (14).
    *           Untere 4 Bits (0) stellen den Teiler auf 1 ein.
    * Diese Einstellung beeinflusst Bildwiederholrate und Stromverbrauch.
    */
    ssd1306_WriteCommand(0xD5); // Setze Taktteiler/Oszillatorfrequenz
    ssd1306_WriteCommand(0xF0); // Teilerverhältnis


    /**
     * @brief Pre-Charge-Periode einstellen.
     *
     * 0xD9 ist der Befehlscode, gefolgt vom Periodenwert.
     * 0x22: Obere 4 Bits (2) stellen die Phase 2 auf 2 DCLKs ein.
     *       Untere 4 Bits (2) stellen die Phase 1 auf 2 DCLKs ein.
     * Beeinflusst die Ladedauer der Pixel und damit Bildqualität und Stromverbrauch.
     */
    ssd1306_WriteCommand(0xD9); // Setze Pre-Charge-Periode
    ssd1306_WriteCommand(0x22);


    /**
    * @brief COM-Pin-Hardwarekonfiguration.
    *
    * 0xDA ist der Befehlscode, gefolgt vom Konfigurationswert.
    * 0x12: Alternative COM-Pin-Konfiguration (Bit 4=1) mit Deaktivierung des
    *       COM-Links/Rechts-Remappings (Bit 5=0) für 128x64 OLED-Displays.
    */
    ssd1306_WriteCommand(0xDA); // Setze COM-Pin-Hardwarekonfiguration
    ssd1306_WriteCommand(0x12);


    /**
     * @brief VCOMH-Entladepegel einstellen.
     *
     * 0xDB ist der Befehlscode, gefolgt vom Spannungspegel.
     * 0x20 entspricht ~0,77xVcc, dem mittleren Wert für VCOMH.
     * Dieser Parameter beeinflusst die Spannung an den COM-Pins und damit
     * die Bildqualität (niedrigere Werte erhöhen Kontrast, aber können Geisterbilder verursachen).
     */
    ssd1306_WriteCommand(0xDB); // Setze VCOMH-Pegel
    ssd1306_WriteCommand(0x20); // 0.77xVcc


    /**
     * @brief DC-DC-Spannungswandler steuern.
     *
     * 0x8D ist der Befehlscode, gefolgt vom DC-DC-Einstellungswert.
     * 0x14 aktiviert den internen DC-DC-Wandler (Bit 2=1), der für die
     * Betriebsspannung des Displays erforderlich ist.
     * 0x10 würde den Wandler deaktivieren (externe Versorgung erforderlich).
     */
    ssd1306_WriteCommand(0x8D); // Aktiviere DC-DC-Wandler
    ssd1306_WriteCommand(0x14);

    ssd1306_SetDisplayOn(1); // Display einschalten

    //https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#page=64

    // Bildschirm löschen
    ssd1306_Fill(Black);

    // Pufferinhalt auf das Display schreiben
    ssd1306_UpdateScreen();

    // Standardwerte für das Bildschirmobjekt setzen
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
}

/* Fill the whole screen with the given color */
void ssd1306_Fill(SSD1306_COLOR color) {
    memset(SSD1306_Buffer, (color == Black) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

/* Write the screenbuffer with changed to the screen */
void ssd1306_UpdateScreen(void) {
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
        ssd1306_WriteCommand(0xB0 + i); // Set the current RAM page address.
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
    }
}

void ssd1306_SetDisplayOn(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SSD1306.DisplayOn = 0;
    }
    ssd1306_WriteCommand(value);
}

void ssd1306_SetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    ssd1306_WriteCommand(kSetContrastControlRegister);
    ssd1306_WriteCommand(value);
}

/**
 * @brief Zeichnet ein einzelnes Pixel in den Bildschirmpuffer.
 *
 * Diese Funktion setzt oder löscht ein Pixel im Bildschirmpuffer basierend auf den
 * angegebenen Koordinaten und der Farbe. Der Bildschirmpuffer wird nicht direkt
 * auf das Display übertragen, sondern muss mit `ssd1306_UpdateScreen()` aktualisiert werden.
 *
 * @param x X-Koordinate des Pixels (0 bis SSD1306_WIDTH-1).
 * @param y Y-Koordinate des Pixels (0 bis SSD1306_HEIGHT-1).
 * @param color Farbe des Pixels (White oder Black).
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Verhindert das Schreiben außerhalb des Puffers
        return;
    }

    // Zeichnet das Pixel in der angegebenen Farbe (White oder Black) in den Bildschirmpuffer.
    // Die Position im Puffer wird durch die Formel `x + (y / 8) * SSD1306_WIDTH` berechnet.
    // - Wenn die Farbe White ist, wird das entsprechende Bit im Puffer gesetzt (1).
    // - Wenn die Farbe Black ist, wird das entsprechende Bit im Puffer gelöscht (0).

    /**
     * Berechnet die Pufferposition für das zu zeichnende Pixel.
     *
     * Das SSD1306 OLED-Display verwendet ein spezielles Speicherlayout:
     * - Der Speicher ist in "Seiten" organisiert, wobei jede Seite 8 vertikale Pixel umfasst
     * - Jedes Byte im Puffer kontrolliert 8 übereinander liegende Pixel (eine vertikale 8-Pixel-Spalte)
     *
     * Die Berechnung erfolgt in zwei Teilen:
     * 1. x + (y / 8) * SSD1306_WIDTH: Bestimmt das korrekte Byte im Puffer
     *    - y / 8: Bestimmt die Seitennummer (0-7 für ein 64-Pixel-Display)
     *    - * SSD1306_WIDTH: Multipliziert mit der Displaybreite, um zur richtigen Seite zu gelangen
     *    - + x: Fügt die horizontale Position hinzu
     *
     * 2. 1 << (y % 8): Bestimmt das richtige Bit innerhalb des gefundenen Bytes
     *    - y % 8: Bestimmt die Position des Pixels innerhalb der 8-Pixel-Spalte (0-7)
     */
    if(color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

/**
 * @brief Positioniert den Cursor für Textausgabe.
 *
 * Diese Funktion setzt die aktuelle X- und Y-Position des Cursors, die für
 * Textausgabe oder andere Zeichenoperationen verwendet wird.
 *
 * @param x X-Koordinate des Cursors.
 * @param y Y-Koordinate des Cursors.
 */
void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

/**
 * @brief Zeichnet eine Linie zwischen zwei Punkten.
 *
 * Diese Funktion verwendet den Bresenham-Algorithmus, um eine Linie zwischen
 * zwei Punkten zu zeichnen. Die Linie wird im Bildschirmpuffer gezeichnet und
 * muss mit `ssd1306_UpdateScreen()` auf das Display übertragen werden.
 *
 * @param x1 X-Koordinate des Startpunkts.
 * @param y1 Y-Koordinate des Startpunkts.
 * @param x2 X-Koordinate des Endpunkts.
 * @param y2 Y-Koordinate des Endpunkts.
 * @param color Farbe der Linie (White oder Black).
 * @see https://de.wikipedia.org/wiki/Bresenham-Algorithmus
 */
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {


    int32_t deltaX = abs(x2 - x1); // Berechnet die absolute Differenz der X-Koordinaten.
    int32_t deltaY = abs(y2 - y1); // Berechnet die absolute Differenz der Y-Koordinaten.
    int32_t signX = ((x1 < x2) ? 1 : -1); // Bestimmt die Richtung der X-Bewegung.
    int32_t signY = ((y1 < y2) ? 1 : -1); // Bestimmt die Richtung der Y-Bewegung.
    int32_t error = deltaX - deltaY; // Initialisiert den Fehlerwert für die Pixelposition.
    int32_t error2;

    ssd1306_DrawPixel(x2, y2, color); // Zeichnet den Endpunkt der Linie.

    while((x1 != x2) || (y1 != y2)) { // Schleife, bis der Startpunkt den Endpunkt erreicht.
        ssd1306_DrawPixel(x1, y1, color); // Zeichnet das aktuelle Pixel.
        error2 = error * 2; // Verdoppelt den Fehlerwert für die nächste Berechnung.
        if(error2 > -deltaY) { // Korrigiert den Fehler in der Y-Richtung.
            error -= deltaY;
            x1 += signX; // Bewegt sich in X-Richtung.
        }

        if(error2 < deltaX) { // Korrigiert den Fehler in der X-Richtung.
            error += deltaX;
            y1 += signY; // Bewegt sich in Y-Richtung.
        }
    }
}

/**
 * @brief Zeichnet ein Rechteck.
 *
 * Diese Funktion zeichnet die vier Seiten eines Rechtecks zwischen den angegebenen
 * Koordinaten. Die Linien werden im Bildschirmpuffer gezeichnet und müssen mit
 * `ssd1306_UpdateScreen()` auf das Display übertragen werden.
 *
 * @param x1 X-Koordinate der oberen linken Ecke.
 * @param y1 Y-Koordinate der oberen linken Ecke.
 * @param x2 X-Koordinate der unteren rechten Ecke.
 * @param y2 Y-Koordinate der unteren rechten Ecke.
 * @param color Farbe des Rechtecks (White oder Black).
 */
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
    ssd1306_Line(x1, y1, x2, y1, color);
    ssd1306_Line(x2, y1, x2, y2, color);
    ssd1306_Line(x2, y2, x1, y2, color);
    ssd1306_Line(x1, y2, x1, y1, color);
}

/**
 * @brief Zeichnet ein gefülltes Rechteck.
 *
 * Diese Funktion füllt ein Rechteck zwischen den angegebenen Koordinaten mit der
 * angegebenen Farbe. Die Pixel werden im Bildschirmpuffer gesetzt und müssen mit
 * `ssd1306_UpdateScreen()` auf das Display übertragen werden.
 *
 * @param x1 X-Koordinate der oberen linken Ecke.
 * @param y1 Y-Koordinate der oberen linken Ecke.
 * @param x2 X-Koordinate der unteren rechten Ecke.
 * @param y2 Y-Koordinate der unteren rechten Ecke.
 * @param color Farbe des Rechtecks (White oder Black).
 */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {

    // Bestimmt die Start- und Endkoordinaten für die X- und Y-Achse unabhängig von der Reihenfolge der Eingabekoordinaten.
    // Dadurch wird sichergestellt, dass die Schleifen korrekt funktionieren, auch wenn x1 > x2 oder y1 > y2.
    uint8_t x_start = ((x1 <= x2) ? x1 : x2); // Kleinere X-Koordinate als Startpunkt.
    uint8_t x_end   = ((x1 <= x2) ? x2 : x1); // Größere X-Koordinate als Endpunkt.
    uint8_t y_start = ((y1 <= y2) ? y1 : y2); // Kleinere Y-Koordinate als Startpunkt.
    uint8_t y_end   = ((y1 <= y2) ? y2 : y1); // Größere Y-Koordinate als Endpunkt.

    // Iteriert über alle Pixel im definierten Rechteckbereich.
    // Die äußere Schleife durchläuft die Y-Koordinaten (Zeilen),
    // während die innere Schleife die X-Koordinaten (Spalten) durchläuft.
    for (uint8_t y = y_start; (y <= y_end) && (y < SSD1306_HEIGHT); y++) {
        for (uint8_t x = x_start; (x <= x_end) && (x < SSD1306_WIDTH); x++) {
            // Zeichnet ein Pixel an der aktuellen Position (x, y) mit der angegebenen Farbe.
            ssd1306_DrawPixel(x, y, color);
        }
    }
}



/**
 * @brief Schreibt ein einzelnes Zeichen in den Bildschirmpuffer.
 *
 * Diese Funktion zeichnet ein einzelnes Zeichen mit der angegebenen Schriftart und Farbe
 * an die aktuelle Cursorposition im Bildschirmpuffer. Die Funktion überprüft, ob das Zeichen
 * gültig ist und ob genügend Platz auf der aktuellen Zeile vorhanden ist. Wenn das Zeichen
 * erfolgreich gezeichnet wurde, wird die Cursorposition entsprechend aktualisiert.
 *
 * @param ch Das zu zeichnende Zeichen (ASCII-Wert zwischen 32 und 126).
 * @param Font Die Schriftart, die für das Zeichnen des Zeichens verwendet wird.
 * @param color Die Farbe des Zeichens (Black oder White).
 * @return Das gezeichnete Zeichen oder 0, wenn das Zeichnen fehlschlägt.
 */
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color) {
    uint32_t i, b, j;

    // Überprüft, ob das Zeichen gültig ist (ASCII-Wert zwischen 32 und 126).
    if (ch < 32 || ch > 126)
        return 0;

    // Ermittelt die Breite des Zeichens basierend auf der Schriftart.
    // Für proportionale Schriftarten wird die Breite aus dem Font-Array abgerufen.
    const uint8_t char_width = Font.char_width ? Font.char_width[ch-32] : Font.width;

    // Überprüft, ob genügend Platz auf der aktuellen Zeile vorhanden ist.
    if (SSD1306_WIDTH < (SSD1306.CurrentX + char_width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height))
    {
        // Nicht genug Platz auf der aktuellen Zeile.
        return 0;
    }

    // Zeichnet das Zeichen Pixel für Pixel basierend auf der Schriftart.
    for(i = 0; i < Font.height; i++) {
        b = Font.data[(ch - 32) * Font.height + i]; // Ruft die Pixel-Daten für die aktuelle Zeile ab.
        for(j = 0; j < char_width; j++) {
            if((b << j) & 0x8000)  {
                // Zeichnet ein Pixel in der angegebenen Farbe.
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            } else {
                // Zeichnet ein Pixel in der inversen Farbe.
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // Aktualisiert die Cursorposition, da der Platz für das Zeichen jetzt belegt ist.
    SSD1306.CurrentX += char_width;

    // Gibt das gezeichnete Zeichen zurück, um den Erfolg zu bestätigen.
    return ch;
}

/**
 * @brief Schreibt eine Zeichenkette in den Bildschirmpuffer.
 *
 * Diese Funktion zeichnet eine Zeichenkette mit der angegebenen Schriftart und Farbe
 * in den Bildschirmpuffer. Die Zeichen werden nacheinander gezeichnet, bis die gesamte
 * Zeichenkette verarbeitet ist oder ein Zeichen nicht gezeichnet werden kann.
 *
 * @param str Die zu zeichnende Zeichenkette.
 * @param Font Die Schriftart, die für das Zeichnen der Zeichen verwendet wird.
 * @param color Die Farbe der Zeichen (Black oder White).
 * @return Das erste Zeichen, das nicht gezeichnet werden konnte, oder '\0', wenn alle Zeichen erfolgreich gezeichnet wurden.
 */
char ssd1306_WriteString(char* str, SSD1306_Font_t Font, SSD1306_COLOR color) {
    while (*str) {
        // Zeichnet das aktuelle Zeichen. Wenn das Zeichnen fehlschlägt, wird das Zeichen zurückgegeben.
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            return *str;
        }
        str++; // Nächstes Zeichen in der Zeichenkette.
    }

    // Gibt '\0' zurück, wenn alle Zeichen erfolgreich gezeichnet wurden.
    return *str;
}
