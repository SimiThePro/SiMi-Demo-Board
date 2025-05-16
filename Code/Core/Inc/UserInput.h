
/**
 * @file UserInput.h
 * @brief Schnittstelle zur Verarbeitung von Benutzereingaben
 *
 * Definiert Funktionen und Datenstrukturen für das UserInput-Modul,
 * das Benutzereingaben (Joystick, Buttons) über GPIO-Pins verarbeitet.
 * Das Modul unterstützt:
 * - Zwei Betriebsmodi: Interrupt- oder Polling-basiert
 * - Entprellung via Delay oder Hardware-Timer
 * - Flankenerkennung für sechs verschiedene Eingabetasten
 *
 * @note Die Implementierung verwendet entweder den Interrupt- oder Polling-Modus,
 * was über Präprozessor-Direktiven konfiguriert wird.
 */

#ifndef CLIONTEST_USERINPUT_H
#define CLIONTEST_USERINPUT_H

#include "main.h"
#include "Pin.h"

/// @brief Enum für die einzelnen Benutzereingaben
enum UserInputs{
    MDS_LEFT,
    MDS_RIGHT,
    MDS_UP,
    MDS_DOWN,
    MDS_BUTTON,
    USER_BUTTON,
    USER_INPUT_NONE
};

/// Kommentiere eine der beiden Zeilen aus um den Interrupt oder Polling zu verwenden
//#define USE_POLLING
#define USE_INTERRUPT

/// Check ob USE_POLLING und USE_INTERRUPT gleichzeitig definiert sind
#if defined(USE_POLLING) && defined(USE_INTERRUPT)
#error "Es darf nur USE_POLLING oder USE_INTERRUPT definiert sein, nicht beide!"
#endif


#ifdef USE_POLLING
void PollingUserInput(void);
#endif

/// Kommentiere eine der beiden Zeilen aus um entweder mit Delay oder Timern zu arbeiten
//#define DEBOUNCE_WITH_DELAY
#define DEBOUNCE_WITH_TIMER

/// Check ob DEBOUNCE_WITH_DELAY und DEBOUNCE_WITH_TIMER gleichzeitig definiert sind
#if defined(DEBOUNCE_WITH_DELAY) && defined(DEBOUNCE_WITH_TIMER)
#error "Es darf nur DEBOUNCE_WITH_DELAY oder DEBOUNCE_WITH_TIMER definiert sein, nicht beide!"
#endif

#if defined(USE_INTERRUPT) && defined(DEBOUNCE_WITH_DELAY)
#error "Es kann nicht USE_INTERRUPT und DEBOUNCE_WITH_TIMER gleichzeitig definiert sein! Weil Delay in den Interrupts nicht funktionieren!"
#endif

#ifdef DEBOUNCE_WITH_TIMER
void HandleDebouncedUserInput(void);
extern volatile uint8_t debounce_in_progress;
#endif

#ifdef USE_INTERRUPT
/// Funktion welche den Interrupt der Pins auslesen
/// @param GPIO_Pin Pin welcher den Interrupt ausgelöst hat (GPIO_PIN0-15)
void UserInput_Interrupt(uint16_t GPIO_Pin);
#endif


void HandlePendingUserInput(void);
void HandleMDSLeft();
void ResetFlanken();


extern volatile uint8_t MDS_LEFT_Flanke ;
extern volatile uint8_t MDS_RIGHT_Flanke;
extern volatile uint8_t MDS_UP_Flanke;
extern volatile uint8_t MDS_DOWN_Flanke;
extern volatile uint8_t MDS_BUTTON_Flanke;
extern volatile uint8_t USER_BUTTON_Flanke;

#endif //CLIONTEST_USERINPUT_H
