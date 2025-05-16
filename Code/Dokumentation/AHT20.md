# AHT20 Sensor-Bibliothek Dokumentation

Diese Bibliothek ermöglicht die einfache Kommunikation mit dem AHT20 Temperatur- und Feuchtigkeitssensor über die I2C-Schnittstelle.

## Überblick

Der AHT20 ist ein industrieller Temperatur- und Feuchtigkeitssensor mit folgenden Eigenschaften:
- Hochpräzise Temperatur- und Feuchtigkeitsmessung
- I2C-Schnittstelle (Adresse: 0x38)
- Messbereich: -40°C bis +85°C / 0% bis 100% relative Luftfeuchtigkeit
- Genauigkeit: ±0.3°C / ±2% RH

## Integration

### Voraussetzungen

- STM32-Mikrocontroller mit HAL-Bibliothek
- Konfigurierte I2C-Schnittstelle (in diesem Beispiel: hi2c2)

### Header-Datei (AHT20.h)

```c
/**
 * @file AHT20.h
 * @brief Schnittstelle zur Kommunikation mit AHT20 Temperatur- und Feuchtigkeitssensor
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#include "main.h"

/**
 * @brief Liest aktuelle Temperatur- und Feuchtigkeitswerte vom AHT20 Sensor
 * @param[out] Temp   Zeiger auf Variable zur Speicherung der Temperatur (-50 bis +150 °C)
 * @param[out] Humid  Zeiger auf Variable zur Speicherung der relativen Luftfeuchtigkeit (0-100%)
 */
void AHT20_Read(float* Temp, float* Humid);

#endif /* INC_AHT20_H_ */
```

### Implementierung (AHT20.c)

Die Implementierung enthält eine Funktion zur Abfrage der Sensor-Werte gemäß dem AHT20-Datenblatt. Sie führt folgende Schritte durch:

1. Prüfen des Kalibrierungsstatus
2. Kalibrierung (falls erforderlich)
3. Starten einer Messung
4. Warten auf Messabschluss
5. Auslesen der Rohdaten
6. Verarbeitung und Umrechnung der Rohdaten in physikalische Größen

Der Sensor verwendet ein spezielles Datenformat:
- 20-Bit-Werte für Temperatur und Luftfeuchtigkeit
- Daten verteilt auf mehrere Bytes
- Mathematische Umrechnung der Rohwerte in physikalische Einheiten

## Verwendungsbeispiel

```c
#include "AHT20.h"

int main(void)
{
    // System-Initialisierung...
    
    float temperatur, luftfeuchtigkeit;
    
    while (1)
    {
        // Sensor auslesen
        AHT20_Read(&temperatur, &luftfeuchtigkeit);
        
        // Mit den Werten arbeiten...
        // z.B. LCD anzeigen, Daten speichern, etc.
        
        HAL_Delay(1000); // Nur einmal pro Sekunde abfragen
    }
}
```

## Technische Hinweise

1. **I2C-Adresse**: Der AHT20 hat die I2C-Adresse 0x38, die in der Bibliothek links um 1 Bit verschoben wird (0x70), da das niederwertigste Bit für das R/W-Flag reserviert ist.

2. **Kalibrierung**: Der Sensor benötigt möglicherweise eine Initialisierung/Kalibrierung beim ersten Einsatz. Die Bibliothek prüft dies automatisch und führt sie bei Bedarf durch.

3. **Messzeit**: Eine vollständige Messung dauert ca. 80ms. Die Bibliothek enthält entsprechende Wartezeiten.

4. **Datenformat**:
    - Temperaturwert wird als 20-Bit-Wert übertragen und in den Bereich -50°C bis +150°C umgerechnet
    - Luftfeuchtigkeit wird als 20-Bit-Wert übertragen und in den Bereich 0-100% umgerechnet

5. **Statusregister**: Das Bit 7 des Statusregisters zeigt an, ob eine Messung läuft (1) oder abgeschlossen ist (0). Bit 3 zeigt den Kalibrierungsstatus an.

## Fehlerbehandlung

Die aktuelle Implementierung enthält keine explizite Fehlerbehandlung. Bei Bedarf können folgende Prüfungen hinzugefügt werden:

- Überprüfung der I2C-Kommunikationsfehler
- Timeout bei zu langer Messzeit
- Plausibilitätsprüfung der Messwerte

## Quellen

Die Implementierung basiert auf dem offiziellen AHT20-Datenblatt:
- [AHT-20 Datenblatt](https://files.seeedstudio.com/wiki/Grove-AHT20_I2C_Industrial_Grade_Temperature_and_Humidity_Sensor/AHT20-datasheet-2020-4-16.pdf)