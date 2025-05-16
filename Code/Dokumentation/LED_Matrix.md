# LED_Matrix-Bibliothek Dokumentation

Diese Bibliothek ermöglicht die Ansteuerung einer 8x8 LED-Matrix über den MAX7219 LED-Treiber-Chip mittels SPI-Schnittstelle.

## Überblick

Die LED_Matrix-Bibliothek bietet Funktionen zur Steuerung einer 8x8 LED-Matrix mit dem MAX7219 Treiber:
- Einfache Initialisierung des MAX7219
- Direktes Zeichnen einzelner Zeilen
- Helligkeitssteuerung (16 Stufen)
- Ein- und Ausschalten der Matrix
- Zurücksetzen der Matrix

## Technische Spezifikationen

- Unterstützt 8x8 LED-Matrix mit MAX7219 Controller
- SPI-Kommunikation (SPI4 in dieser Implementierung)
- Maximale SPI-Taktrate: 10 MHz (laut MAX7219 Datenblatt)

## API-Referenz

### Initialisierung

```c
void LED_Matrix_setup(void);
```
Initialisiert die LED-Matrix mit Standardeinstellungen:
- Dekodierungsmodus deaktiviert
- Scan-Limit auf alle Zeilen (0-7)
- Standard-Helligkeitsstufe 2
- Aktiviert den normalen Betriebsmodus

### Betriebsmodus einstellen

```c
void LED_Matrix_set_mode(uint8_t mode);
```
Setzt den Betriebsmodus der LED-Matrix:
- `mode = 0`: Shutdown-Modus (Display aus, minimaler Stromverbrauch)
- `mode = 1`: Normalbetrieb (Display aktiv)

### Helligkeit einstellen

```c
void LED_Matrix_set_intensity(uint8_t intensity);
```
Stellt die Helligkeit der LED-Matrix ein:
- `intensity`: 0-15, wobei 0 die niedrigste und 15 die höchste Helligkeit darstellt
- Werte über 15 werden auf 15 begrenzt

### Zeichenfunktionen

```c
void LED_Matrix_draw_row(uint8_t row, uint8_t data);
```
Zeichnet eine einzelne Zeile auf der LED-Matrix:
- `row`: Zeilennummer (0-7, 0 = oberste Zeile)
- `data`: 8-Bit-Wert, jedes Bit entspricht einer LED in der Zeile
    - Bit 0 entspricht der rechten LED
    - Bit 7 entspricht der linken LED
    - `1` = LED ein, `0` = LED aus

### Zurücksetzen

```c
void LED_Matrix_reset(void);
```
Setzt die LED-Matrix zurück:
- Löscht alle Zeilen (setzt alle LEDs auf aus)
- Aktiviert den normalen Betriebsmodus

### Interne Hilfsfunktionen

```c
void LED_Matrix_send_command(uint8_t address, uint8_t data);
```
Sendet einen Befehl an die LED-Matrix:
- `address`: Register-Adresse (gemäß MAX7219-Datenblatt)
- `data`: Zu sendende Daten

```c
void LEDM_CS_H(void);
void LEDM_CS_L(void);
```
Steuern das Chip-Select-Signal für die SPI-Kommunikation.

## Verwendungsbeispiel

```c
#include "LED_Matrix.h"

int main(void)
{
    // System-Initialisierung (Takte, SPI, etc.)
    
    // LED-Matrix initialisieren
    LED_Matrix_setup();
    
    // Helligkeit einstellen (mittlere Helligkeit)
    LED_Matrix_set_intensity(7);
    
    // Einige Muster zeichnen
    
    // Horizontale Linie in Zeile 3
    LED_Matrix_draw_row(3, 0xFF);  // Alle LEDs in Zeile 3 einschalten
    
    // Vertikale Linie
    for(int i = 0; i < 8; i++) {
        LED_Matrix_draw_row(i, 0x10);  // Schaltet die 5. LED von rechts in jeder Zeile ein
    }
    
    // Ein "X" zeichnen
    LED_Matrix_draw_row(0, 0x81);  // 10000001
    LED_Matrix_draw_row(1, 0x42);  // 01000010
    LED_Matrix_draw_row(2, 0x24);  // 00100100
    LED_Matrix_draw_row(3, 0x18);  // 00011000
    LED_Matrix_draw_row(4, 0x18);  // 00011000
    LED_Matrix_draw_row(5, 0x24);  // 00100100
    LED_Matrix_draw_row(6, 0x42);  // 01000010
    LED_Matrix_draw_row(7, 0x81);  // 10000001
    
    while(1) {
        // Hauptschleife
    }
}
```

## Hardware-Anschluss

Der MAX7219 wird über die SPI-Schnittstelle angesteuert:
- `MOSI`: Datenleitung (Master Out Slave In)
- `SCK`: Takt
- `CS` (Chip Select): Aktiviert die Kommunikation mit dem MAX7219
    - Muss vor der Übertragung auf LOW gezogen werden
    - Nach der Übertragung wieder auf HIGH setzen

Die LED-Matrix wird über die folgenden Pins des MAX7219 angeschlossen:
- DIG0-DIG7: Steuerung der einzelnen Zeilen (Kathoden)
- SEGA-SEGG: Steuerung der einzelnen Spalten (Anoden)

## Technische Hinweise

1. **SPI-Timing**: Die maximale SPI-Taktfrequenz für den MAX7219 beträgt 10 MHz. Höhere Frequenzen können zu instabilem Betrieb führen.

2. **Stromverbrauch**: Die Helligkeitseinstellung beeinflusst direkt den Stromverbrauch. Bei voller Helligkeit (15) und allen LEDs eingeschaltet kann der Stromverbrauch erheblich ansteigen.

3. **Adressierung**: Die Bibliothek verwendet die Register-Adressierung gemäß dem MAX7219-Datenblatt:
    - Register 0x01-0x08: Zeilendaten (Digit 0-7)
    - Register 0x09: Dekodierungsmodus
    - Register 0x0A: Helligkeitsregister
    - Register 0x0B: Scan-Limit-Register
    - Register 0x0C: Shutdown-Register

4. **Registerwerte**: Es werden bewusst keine Dekodierungsmodi verwendet, da diese für 7-Segment-Anzeigen gedacht sind und für LED-Matrizen nicht geeignet sind.

5. **Datenformat**: Beim Zeichnen einer Zeile entspricht das LSB (Bit 0) der rechten LED und das MSB (Bit 7) der linken LED in der Zeile.

## Quellen und weitere Informationen

Die Implementierung basiert auf dem offiziellen MAX7219/MAX7221-Datenblatt:
https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf