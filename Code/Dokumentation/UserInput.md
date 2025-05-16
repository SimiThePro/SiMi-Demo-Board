# Dokumentation: UserInput-Modul

Diese Bibliothek ermöglicht die Verarbeitung von Benutzereingaben über verschiedene Eingabegeräte (Joystick, Buttons). Sie bietet flexible Konfigurationsmöglichkeiten für unterschiedliche Anwendungsszenarien.

## Technische Spezifikation

Das UserInput-Modul bietet:
- Unterstützung für 6 Eingaberichtungen/Buttons
- Zwei Betriebsmodi: Interrupt- oder Polling-basiert
- Entprellungsmechanismen: Delay-basiert oder Timer-basiert
- Flankenerkennung für alle Eingaben

## Konfigurationsoptionen

Die Bibliothek kann über Präprozessor-Direktiven konfiguriert werden:

1. **Betriebsmodus**:
   ```c
   // Nur eine Option aktivieren
   //#define USE_POLLING     // Polling-basierte Eingabeverarbeitung
   #define USE_INTERRUPT     // Interrupt-basierte Eingabeverarbeitung
   ```

2. **Entprellungsmethode**:
   ```c
   // Nur eine Option aktivieren
   //#define DEBOUNCE_WITH_DELAY  // Entprellung über Delays
   #define DEBOUNCE_WITH_TIMER    // Entprellung über Hardware-Timer
   ```

## Unterstützte Eingaben

Die Bibliothek unterstützt folgende Eingaben (definiert im `UserInputs` Enum):
- `MDS_LEFT` - Joystick links
- `MDS_RIGHT` - Joystick rechts
- `MDS_UP` - Joystick hoch
- `MDS_DOWN` - Joystick runter
- `MDS_BUTTON` - Joystick-Button
- `USER_BUTTON` - Zusätzlicher Benutzer-Button
- `USER_INPUT_NONE` - Keine Eingabe

## Hauptfunktionen

### Im Polling-Modus
```c
void PollingUserInput(void);
```
Überprüft regelmäßig den Status der Eingabe-Pins und aktualisiert die Flankenerkennungs-Flags.

### Im Interrupt-Modus
```c
void UserInput_Interrupt(uint16_t GPIO_Pin);
```
Callback-Funktion zur Verarbeitung von GPIO-Interrupts. Diese Funktion sollte in der HAL-GPIO-Interrupt-Callback-Funktion aufgerufen werden.

### Bei Timer-basierter Entprellung
```c
void HandleDebouncedUserInput(void);
```
Verarbeitet die entprellten Eingaben nach Ablauf der Timer-Zeitüberschreitung.

### Allgemeine Funktionen
```c
void HandlePendingUserInput(void);   // Verarbeitet ausstehende Benutzereingaben
void HandleMDSLeft();                // Beispiel einer Verarbeitungsfunktion für eine spezifische Eingabe
void ResetFlanken();                 // Setzt alle Flankenerkennnungs-Flags zurück
```

## Status-Flags

Die Bibliothek stellt Flankenerkennungs-Flags für jede Eingabe bereit:
```c
extern volatile uint8_t MDS_LEFT_Flanke;
extern volatile uint8_t MDS_RIGHT_Flanke;
extern volatile uint8_t MDS_UP_Flanke;
extern volatile uint8_t MDS_DOWN_Flanke;
extern volatile uint8_t MDS_BUTTON_Flanke;
extern volatile uint8_t USER_BUTTON_Flanke;
```

Diese Flags werden gesetzt, wenn eine entsprechende Eingabe erkannt wurde, und können in der Anwendung abgefragt werden.

## Verwendungsbeispiel

### Interrupt-Modus mit Timer-Entprellung

```c
// In der STM32 HAL GPIO-Callback-Funktion
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    UserInput_Interrupt(GPIO_Pin);
}

// Im Timer-Interrupt oder periodischem Aufruf
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        if (debounce_in_progress) {
            HandleDebouncedUserInput();
        }
    }
}

// In der Hauptschleife
void main(void)
{
    // Initialisierungen...
    
    while (1) {
        // Prüfen und verarbeiten ausstehender Eingaben
        HandlePendingUserInput();
        
        // Wenn links gedrückt wurde
        if (MDS_LEFT_Flanke) {
            // Reaktion auf Linksbewegung
            MDS_LEFT_Flanke = 0;  // Flag zurücksetzen
        }
        
        // Weitere Verarbeitung...
    }
}
```

## Hinweise

- Die Konfiguration `USE_INTERRUPT` und `DEBOUNCE_WITH_DELAY` zusammen ist nicht erlaubt, da Delays in Interrupt-Routinen nicht verwendet werden sollten
- Bei Verwendung des Timer-Debounce-Mechanismus muss ein externer Timer konfiguriert werden
- Die Implementierung verwendet Flankenerkennung für stabile Signalverarbeitung
- Die Funktionen `HandlePendingUserInput()` und `HandleMDSLeft()` müssen in der Implementierung entsprechend dem Anwendungsfall angepasst werden