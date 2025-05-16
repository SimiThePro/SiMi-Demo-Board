/******************************************************************************
 *
 *                       DOKUMENTATION: UserInput-Modul
 *
 ******************************************************************************/

/******************************************************************************
 * BESCHREIBUNG
 ******************************************************************************/
/* Dieses Modul stellt eine vollständige Implementierung zur Erfassung und
 * Verarbeitung von Benutzereingaben auf einem STM32-basierten System bereit.
 * Es unterstützt verschiedene Betriebsmodi und Entprellmethoden, um flexibel
 * an die Anforderungen des jeweiligen Projekts angepasst werden zu können.
 */

/******************************************************************************
 * KONFIGURATION
 ******************************************************************************/
/* Die folgenden Konfigurationsoptionen können in UserInput.h definiert werden:
 *
 * Betriebsmodus:
 * - USE_POLLING: Aktiviert den Polling-Modus (regelmäßige Pin-Abfrage)
 * - USE_INTERRUPT: Aktiviert den Interrupt-Modus (event-gesteuerte Verarbeitung)
 *
 * Entprellmethode:
 * - DEBOUNCE_WITH_DELAY: Verwendet HAL_Delay für die Entprellung
 * - DEBOUNCE_WITH_TIMER: Verwendet Timer-Interrupts (TIM6) für die Entprellung
 */

/******************************************************************************
 * BENUTZUNG
 ******************************************************************************/
/* 1. Initialisierung:
 *    - GPIO-Pins für die Buttons konfigurieren
 *    - Bei USE_INTERRUPT: Externe Interrupts für die Pins aktivieren
 *    - Bei DEBOUNCE_WITH_TIMER: TIM6-Timer konfigurieren
 *
 * 2. Hauptschleife im Polling-Modus:
 *    while (1) {
 *        PollingUserInput();          // Erfasst Benutzereingaben
 *        HandlePendingUserInput();    // Überträgt Flankenereignisse
 *
 *        // Überprüfen der Flankenvariablen
 *        if (MDS_UP_Flanke) {
 *            // Aktion für MDS_UP ausführen
 *            ResetFlanken();          // Flanken zurücksetzen
 *        }
 *    }
 *
 * 3. Hauptschleife im Interrupt-Modus:
 *    while (1) {
 *        HandleMDSLeft();             // MDS_LEFT per Polling abfragen
 *        HandlePendingUserInput();    // Überträgt Flankenereignisse
 *
 *        // Überprüfen der Flankenvariablen
 *        if (MDS_DOWN_Flanke) {
 *            // Aktion für MDS_DOWN ausführen
 *            ResetFlanken();          // Flanken zurücksetzen
 *        }
 *    }
 */

/******************************************************************************
 * FUNKTIONEN
 ******************************************************************************/
/* Öffentliche Funktionen:
 *
 * - PollingUserInput(): Erfasst Eingaben im Polling-Modus
 *   Aufruf: Regelmäßig in der Hauptschleife (nur bei USE_POLLING)
 *
 * - HandlePendingUserInput(): Überträgt anstehende Flanken in globale Variablen
 *   Aufruf: Regelmäßig in der Hauptschleife
 *
 * - ResetFlanken(): Setzt alle Flankenvariablen zurück
 *   Aufruf: Nach der Verarbeitung erkannter Flanken
 *
 * - HandleMDSLeft(): Spezialbehandlung für MDS_LEFT-Eingabe
 *   Aufruf: Regelmäßig in der Hauptschleife (im Interrupt-Modus)
 *
 * - UserInput_Interrupt(): ISR für GPIO-Interrupts
 *   Aufruf: Wird automatisch von HAL_GPIO_EXTI_Callback() aufgerufen
 */

/******************************************************************************
 * BESONDERHEITEN
 ******************************************************************************/
/* - Die MDS_LEFT-Taste wird immer per Polling abgefragt, auch im Interrupt-Modus,
 *   da MDS_LEFT und MDS_RIGHT den gleichen GPIO-Pin (Pin 5) teilen.
 *
 * - Das Modul verwendet einen zweistufigen Prozess mit "Pending"-Flags, um
 *   Race-Conditions zwischen Interrupt-Kontext und Hauptprogramm zu vermeiden.
 *
 * - Zustandsvariablen sind als `volatile` deklariert für die korrekte
 *   Verwendung in Interrupt-Kontexten.
 *
 * - Bei DEBOUNCE_WITH_TIMER wird TIM6 für die zeitgesteuerte Entprellung verwendet.
 *   Die Timer-ISR ruft HandleDebouncedUserInput() auf.
 */

#include "UserInput.h"

#ifdef USE_POLLING

/// Variable um den vorherigen Status der einzelnen Pins zu speichern
uint8_t MDS_RIGHT_LetzterStatus = 0;
uint8_t MDS_UP_LetzterStatus = 0;
uint8_t MDS_DOWN_LetzterStatus = 0;
uint8_t MDS_BUTTON_LetzterStatus = 0;
uint8_t USER_BUTTON_LetzterStatus = 0;

uint8_t MDS_LEFT_Status = 0;
uint8_t MDS_RIGHT_Status = 0;
uint8_t MDS_UP_Status = 0;
uint8_t MDS_DOWN_Status = 0;
uint8_t MDS_BUTTON_Status = 0;
uint8_t USER_BUTTON_Status = 0;


#ifdef DEBOUNCE_WITH_TIMER


#endif
#endif

#ifdef USE_INTERRUPT

#endif

uint8_t MDS_LEFT_LetzterStatus = 0;


volatile uint8_t Pending_MDS_LEFT_Flanke = 0;
volatile uint8_t Pending_MDS_RIGHT_Flanke = 0;
volatile uint8_t Pending_MDS_UP_Flanke = 0;
volatile uint8_t Pending_MDS_DOWN_Flanke = 0;
volatile uint8_t Pending_MDS_BUTTON_Flanke = 0;
volatile uint8_t Pending_USER_BUTTON_Flanke = 0;

/// Variable welche den aktuellen Status der Pins speichert
volatile uint8_t MDS_LEFT_Flanke = 0;
volatile uint8_t MDS_RIGHT_Flanke = 0;
volatile uint8_t MDS_UP_Flanke = 0;
volatile uint8_t MDS_DOWN_Flanke = 0;
volatile uint8_t MDS_BUTTON_Flanke = 0;
volatile uint8_t USER_BUTTON_Flanke = 0;

/**
 * @brief Speichert Informationen zur zuletzt erkannten Benutzereingabe
 *
 * Diese Variable enthält die Pin-Informationen (GPIO-Port und Pin-Nummer)
 * der zuletzt erkannten Benutzereingabe. Sie wird verwendet, um die
 * Verarbeitung der Eingabe nach einem Entprellvorgang zu ermöglichen.
 */
Pin LetzterUserInputPin;

/**
 * @brief Enum-Wert für die zuletzt erkannte Benutzereingabe
 *
 * Diese Variable speichert den Typ der zuletzt erkannten Benutzereingabe
 * (z. B. MDS_UP, MDS_BUTTON, usw.). Sie wird verwendet, um die Flanke
 * nach einem Entprellvorgang zu identifizieren und zu verarbeiten.
 */
enum UserInputs LetzterUserInput = USER_INPUT_NONE;

extern TIM_HandleTypeDef htim6;

volatile uint8_t debounce_in_progress = 0;
/**
 * @brief Behandelt Flankenereignisse für Benutzereingaben mit Entprellung.
 *
 * Diese Funktion implementiert die Verarbeitung von erkannten Flanken bei
 * Benutzereingaben (Buttons) und kümmert sich um die Entprellung je nach
 * konfiguriertem Modus (Delay oder Timer).
 *
 * @param pin Struktur mit GPIO-Port und Pin-Nummer des betroffenen Pins.
 * @param userInput Enum-Wert, der identifiziert, welche Benutzereingabe erkannt wurde.
 * @param flankenVariable Zeiger auf die Flag-Variable, die bei bestätigter Flanke gesetzt wird.
 *
 * Abhängig von der Konfiguration:
 * - Bei DEBOUNCE_WITH_DELAY: Wartet für einen bestimmten Zeitraum und prüft dann erneut.
 * - Bei DEBOUNCE_WITH_TIMER: Startet einen Timer-Interrupt und speichert die
 *   Pin-Informationen für die spätere Auswertung in HandleDebouncedUserInput().
 */
void HandleFlanke(Pin pin, enum UserInputs userInput,volatile uint8_t* flankenVariable) {
#ifdef DEBOUNCE_WITH_DELAY
     HAL_Delay(50);
     if (HAL_GPIO_ReadPin(pin.GPIOx,pin.GPIO_Pin) == 1 || (HAL_GPIO_ReadPin(pin.GPIOx,pin.GPIO_Pin) == 0 && userInput == USER_BUTTON)) {
          *flankenVariable = 1;
     }

#endif
#ifdef DEBOUNCE_WITH_TIMER
     debounce_in_progress = 1;
     LetzterUserInputPin = (Pin){pin.GPIOx,pin.GPIO_Pin};
     LetzterUserInput = userInput;

     ///Starte den Debounce Timer
     __HAL_TIM_SET_COUNTER(&htim6, 0);
     HAL_TIM_Base_Start_IT(&htim6);
#endif
}

#ifdef USE_POLLING
/**
 * @brief Erfasst und verarbeitet Benutzereingaben im Polling-Modus
 *
 * Diese Funktion überprüft zyklisch alle Eingabe-Pins auf Zustandsänderungen
 * und erkennt dadurch Tastendrücke. Bei erkannten Flanken wird die Entprellung
 * über die Funktion HandleFlanke eingeleitet.
 *
 * Die Funktion:
 * 1. Prüft, ob bereits ein Entprellvorgang läuft und kehrt in diesem Fall sofort zurück
 * 2. Liest den aktuellen Status aller Eingabe-Pins
 * 3. Setzt alle Flankenvariablen zurück
 * 4. Vergleicht den aktuellen mit dem vorherigen Zustand, um Flanken zu erkennen
 * 5. Ruft bei erkannter Flanke die HandleFlanke()-Funktion auf
 * 6. Speichert den aktuellen Status für den nächsten Aufruf
 *
 * @note Diese Funktion sollte regelmäßig im Hauptprogramm aufgerufen werden,
 *       wenn USE_POLLING definiert ist.
 *
 * @see HandleFlanke()
 * @see HandleDebouncedUserInput()
 */
void PollingUserInput(void) {

#ifdef DEBOUNCE_WITH_TIMER
if (debounce_in_progress) return;
#endif

     /// Lese den aktuellen Status der Pins aus
     MDS_LEFT_Status = HAL_GPIO_ReadPin(MDS_LEFT_GPIO_Port,MDS_LEFT_Pin);
     MDS_RIGHT_Status = HAL_GPIO_ReadPin(MDS_RIGHT_GPIO_Port,MDS_RIGHT_Pin);
     MDS_UP_Status = HAL_GPIO_ReadPin(MDS_UP_GPIO_Port,MDS_UP_Pin);
     MDS_DOWN_Status = HAL_GPIO_ReadPin(MDS_DOWN_GPIO_Port,MDS_DOWN_Pin);
     MDS_BUTTON_Status = HAL_GPIO_ReadPin(MDS_BUTTON_GPIO_Port,MDS_BUTTON_Pin);
     USER_BUTTON_Status = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port,USER_BUTTON_Pin);


     /// Kontrolliere Flanke von den Pins
     if (MDS_LEFT_Status == GPIO_PIN_SET && MDS_LEFT_LetzterStatus == 0) {
          ///MDS_LEFT Flanke erkannt
          HandleFlanke((Pin){MDS_LEFT_GPIO_Port,MDS_LEFT_Pin},MDS_LEFT,&MDS_LEFT_Flanke);
     }
     else if (MDS_RIGHT_Status == GPIO_PIN_SET && MDS_RIGHT_LetzterStatus == 0) {
          ///MDS_RIGHT Flanke erkannt
          HandleFlanke((Pin){MDS_RIGHT_GPIO_Port,MDS_RIGHT_Pin},MDS_RIGHT,&MDS_RIGHT_Flanke);
     }
     else if (MDS_UP_Status == GPIO_PIN_SET && MDS_UP_LetzterStatus == 0) {
          ///MDS_UP Flanke erkannt
          HandleFlanke((Pin){MDS_UP_GPIO_Port,MDS_UP_Pin},MDS_UP,&MDS_UP_Flanke);
     }
     else if (MDS_DOWN_Status == GPIO_PIN_SET && MDS_DOWN_LetzterStatus == 0) {
          ///MDS_DOWN Flanke erkannt
          HandleFlanke((Pin){MDS_DOWN_GPIO_Port,MDS_DOWN_Pin},MDS_DOWN,&MDS_DOWN_Flanke);
     }
     else if (MDS_BUTTON_Status == GPIO_PIN_SET && MDS_BUTTON_LetzterStatus == 0) {
          ///MDS_BUTTON Flanke erkannt
          HandleFlanke((Pin){MDS_BUTTON_GPIO_Port,MDS_BUTTON_Pin},MDS_BUTTON,&MDS_BUTTON_Flanke);
     }
     else if (USER_BUTTON_Status == GPIO_PIN_RESET && USER_BUTTON_LetzterStatus == 1) {
          ///USER_BUTTON Flanke erkannt
          HandleFlanke((Pin){USER_BUTTON_GPIO_Port,USER_BUTTON_Pin},USER_BUTTON,&USER_BUTTON_Flanke);
     }

     /// Speichere den aktuellen Status in die LetzterStatus Variablen
     MDS_LEFT_LetzterStatus = MDS_LEFT_Status;
     MDS_RIGHT_LetzterStatus = MDS_RIGHT_Status;
     MDS_UP_LetzterStatus = MDS_UP_Status;
     MDS_DOWN_LetzterStatus = MDS_DOWN_Status;
     MDS_BUTTON_LetzterStatus = MDS_BUTTON_Status;
     USER_BUTTON_LetzterStatus = USER_BUTTON_Status;

}
#endif
/**
 * @brief Verarbeitet die entprellten Benutzereingaben nach Timer-Ablauf
 *
 * Diese Funktion wird vom Timer-Interrupt (TIM6) nach Ablauf der Entprellzeit aufgerufen.
 * Sie überprüft, ob die zuletzt erkannte Benutzereingabe immer noch aktiv ist, und setzt
 * gegebenenfalls die entsprechende Flanken-Variable.
 *
 * Die Funktion:
 * 1. Überprüft den Status des zuletzt erkannten Eingabe-Pins
 * 2. Wenn der Pin immer noch aktiv ist (HIGH), wird die entsprechende Flanken-Variable gesetzt
 * 3. Andernfalls werden die Informationen zur letzten Eingabe zurückgesetzt
 *
 * @note Diese Funktion ist nur verfügbar, wenn DEBOUNCE_WITH_TIMER definiert ist
 * @note Sie wird automatisch vom Timer-Interrupt in HAL_TIM_PeriodElapsedCallback() aufgerufen
 *
 * @see HandleFlanke()
 * @see PollingUserInput()
 * @see HAL_TIM_PeriodElapsedCallback()
 */
void HandleDebouncedUserInput(void) {

     if (HAL_GPIO_ReadPin(LetzterUserInputPin.GPIOx,LetzterUserInputPin.GPIO_Pin) == GPIO_PIN_SET) {
          /// Wenn der Pin immer noch HIGH ist, dann wurde die Flanke erkannt
          switch (LetzterUserInput) {
          case MDS_LEFT:
               Pending_MDS_LEFT_Flanke = 1;
               break;
          case MDS_RIGHT:
               Pending_MDS_RIGHT_Flanke = 1;
               break;
          case MDS_UP:
               Pending_MDS_UP_Flanke = 1;
               break;
          case MDS_DOWN:
               Pending_MDS_DOWN_Flanke = 1;
               break;
          case MDS_BUTTON:
               Pending_MDS_BUTTON_Flanke = 1;
               break;
          default:
               break;
          }
     }
     // Der USER_BUTTON ist gedrückt, wenn der Pin LOW ist
     else if (LetzterUserInput == USER_BUTTON && HAL_GPIO_ReadPin(LetzterUserInputPin.GPIOx,LetzterUserInputPin.GPIO_Pin) == GPIO_PIN_RESET) {
          Pending_USER_BUTTON_Flanke = 1;
     }
     else {
          /// Wenn der Pin nicht mehr HIGH ist, dann wurde die Flanke nicht erkannt
          LetzterUserInput = USER_INPUT_NONE;
          LetzterUserInputPin = (Pin){0,0};
     }
}

/**
 * @brief Überträgt anstehende Flankenereignisse in die globalen Flankenvariablen
 *
 * Diese Funktion überprüft, ob für die verschiedenen Benutzereingaben
 * (z. B. MDS_LEFT, MDS_RIGHT, usw.) anstehende Flankenereignisse vorhanden sind.
 * Wenn ein solches Ereignis erkannt wird, wird die entsprechende globale
 * Flankenvariable gesetzt und das Pending-Flag zurückgesetzt.
 *
 * Die Funktion ist notwendig, um Race-Conditions zwischen Interrupt-Kontext und
 * Hauptprogramm zu vermeiden, da Flanken nicht direkt im Interrupt-Kontext gesetzt
 * werden sollten. Stattdessen werden sie als "pending" markiert und erst hier
 * im Hauptprogramm tatsächlich aktiviert.
 *
 *
 * Die Funktion:
 * 1. Prüft die Pending-Flags für jede Benutzereingabe.
 * 2. Setzt die zugehörige globale Flankenvariable, wenn ein Pending-Flag aktiv ist.
 * 3. Löscht das Pending-Flag nach der Verarbeitung.
 *
 * @note Diese Funktion sollte regelmäßig aufgerufen werden, um anstehende
 *       Flankenereignisse zu verarbeiten.
 */

void HandlePendingUserInput(void) {
     if (Pending_MDS_LEFT_Flanke) {
          MDS_LEFT_Flanke = 1;
          Pending_MDS_LEFT_Flanke = 0;
     }
     if (Pending_MDS_RIGHT_Flanke) {
          MDS_RIGHT_Flanke = 1;
          Pending_MDS_RIGHT_Flanke = 0;
     }
     if (Pending_MDS_UP_Flanke) {
          MDS_UP_Flanke = 1;
          Pending_MDS_UP_Flanke = 0;
     }
     if (Pending_MDS_DOWN_Flanke) {
          MDS_DOWN_Flanke = 1;
          Pending_MDS_DOWN_Flanke = 0;
     }
     if (Pending_MDS_BUTTON_Flanke) {
          MDS_BUTTON_Flanke = 1;
          Pending_MDS_BUTTON_Flanke = 0;
     }
     if (Pending_USER_BUTTON_Flanke) {
          USER_BUTTON_Flanke = 1;
          Pending_USER_BUTTON_Flanke = 0;
     }
}

void ResetFlanken() {
     /// Resettiere die Flanken, damit sie nur einmal erkannt werden
     MDS_LEFT_Flanke = 0;
     MDS_RIGHT_Flanke = 0;
     MDS_UP_Flanke = 0;
     MDS_DOWN_Flanke = 0;
     MDS_BUTTON_Flanke = 0;
     USER_BUTTON_Flanke = 0;
}

#ifdef USE_INTERRUPT

/**
 * @brief Verarbeitet Benutzereingaben basierend auf dem übergebenen UserInput-Wert
 *
 * Diese Funktion wird verwendet, um Flankenereignisse für einen bestimmten
 * Benutzereingabe-Typ zu behandeln. Sie überprüft, ob ein Entprellvorgang
 * bereits läuft, und ruft andernfalls die Funktion HandleFlanke() auf, um
 * die Flanke zu verarbeiten.
 *
 * Die Funktion:
 * 1. Prüft, ob bereits ein Entprellvorgang aktiv ist und kehrt in diesem Fall zurück
 * 2. Verwendet eine switch-Anweisung, um den übergebenen UserInput-Wert auszuwerten
 * 3. Ruft für jeden erkannten Eingabetyp HandleFlanke() mit den entsprechenden Parametern auf
 *
 * @param userInput Enum-Wert, der den Typ der Benutzereingabe angibt (MDS_UP, MDS_BUTTON, usw.)
 *
 * @note Diese Funktion wird im Interrupt-Modus von UserInput_Interrupt() aufgerufen,
 *       wenn ein entsprechendes GPIO-Event erkannt wurde
 *
 * @see HandleFlanke()
 * @see UserInput_Interrupt()
 */
void HandleUserInputInterrupt(enum UserInputs userInput) {

#ifdef DEBOUNCE_WITH_TIMER
     if (debounce_in_progress) return;
#endif

     switch (userInput) {
          case MDS_UP:
               HandleFlanke((Pin){MDS_UP_GPIO_Port,MDS_UP_Pin},MDS_UP,&Pending_MDS_UP_Flanke);
          break;
          case MDS_BUTTON:
               HandleFlanke((Pin){MDS_BUTTON_GPIO_Port,MDS_BUTTON_Pin},MDS_BUTTON,&Pending_MDS_BUTTON_Flanke);
          break;
          case MDS_RIGHT:
               HandleFlanke((Pin){MDS_RIGHT_GPIO_Port,MDS_RIGHT_Pin},MDS_RIGHT,&Pending_MDS_RIGHT_Flanke);
          break;
          case MDS_DOWN:
               HandleFlanke((Pin){MDS_DOWN_GPIO_Port,MDS_DOWN_Pin},MDS_DOWN,&Pending_MDS_DOWN_Flanke);
          break;
          case USER_BUTTON:
               HandleFlanke((Pin){USER_BUTTON_GPIO_Port,USER_BUTTON_Pin},USER_BUTTON,&Pending_USER_BUTTON_Flanke);
          break;
          default:
               break;
     }

}


/**
 * @brief Interrupt-Service-Routine für Benutzereingaben
 *
 * Diese Funktion wird aufgerufen, wenn ein GPIO-Interrupt ausgelöst wird.
 * Sie identifiziert den betroffenen Pin und leitet die Verarbeitung der
 * entsprechenden Benutzereingabe ein.
 *
 * Die Funktion:
 * 1. Überprüft, welcher GPIO-Pin den Interrupt ausgelöst hat
 * 2. Ruft die entsprechende Handler-Funktion für die erkannte Benutzereingabe auf
 *
 * @param GPIO_Pin Die Pin-Nummer des GPIO-Pins, der den Interrupt ausgelöst hat
 *
 * @note MDS_LEFT ist nicht als Interrupt-Auslöser implementiert, da MDS_LEFT und MDS_RIGHT den
 *       gleichen GPIO-Pin (GPIO-Pin 5) teilen. Daher wird MDS_LEFT separat per Polling in HandleMDSLeft() behandelt.
 * @note Diese Funktion wird im Kontext eines GPIO-Interrupts aufgerufen
 * @note Die Zuordnung der GPIO-Pins zu den Benutzereingaben erfolgt über die
 *       in der Funktion implementierte `switch`-Anweisung
 *
 * @see HandleUserInputInterrupt()
 */
void UserInput_Interrupt(uint16_t GPIO_Pin) {
     switch (GPIO_Pin) {
          case GPIO_PIN_15:{ //MDS_UP
               HandleUserInputInterrupt(MDS_UP);
               break;
          }
          case GPIO_PIN_14:{ //MDS_PRESS
               HandleUserInputInterrupt(MDS_BUTTON);
               break;
          }
          case GPIO_PIN_5:{ //MDS_RIGHT
               HandleUserInputInterrupt(MDS_RIGHT);
               break;
          }
          case GPIO_PIN_10:{ //MDS_DOWN
               HandleUserInputInterrupt(MDS_DOWN);
               break;
          }
          case GPIO_PIN_13:{ //USER_BUTTON
               HandleUserInputInterrupt(USER_BUTTON);
               break;
          }
     }
}


/**
 * @brief Behandelt Tastenereignisse für den linken Joystick-Knopf (MDS_LEFT) im Polling-Modus
 *
 * Diese Funktion wird verwendet, da MDS_LEFT und MDS_RIGHT auf der selben GPIO-Pin-Nummer
 * gemappt sind und daher nicht beide gleichzeitig einen Interrupt auslösen können.
 * MDS_LEFT muss daher per Polling abgefragt werden.
 *
 * Die Funktion:
 * 1. Liest den aktuellen Status des MDS_LEFT Pins
 * 2. Erkennt steigende Flanken (Übergang von LOW zu HIGH)
 * 3. Initiiert bei erkannter Flanke den Entprellvorgang mittels Timer
 * 4. Speichert Informationen zur späteren Verarbeitung in HandleDebouncedUserInput()
 * 5. Aktualisiert den gespeicherten vorherigen Zustand für den nächsten Aufruf
 *
 * @note Diese Funktion sollte regelmäßig aufgerufen werden, um den MDS_LEFT Pin zu überwachen
 * @note Wird im Kontext des USE_INTERRUPT Modus verwendet, ergänzt die Interrupt-Handler
 *
 * @see HandleFlanke()
 * @see HandleDebouncedUserInput()
 */
void HandleMDSLeft() {
     GPIO_PinState currentState = HAL_GPIO_ReadPin(MDS_LEFT_GPIO_Port, MDS_LEFT_Pin);
     if (currentState == GPIO_PIN_SET && MDS_LEFT_LetzterStatus == GPIO_PIN_RESET){
          if (!debounce_in_progress){
               HandleFlanke((Pin){MDS_LEFT_GPIO_Port,MDS_LEFT_Pin},MDS_LEFT,&MDS_LEFT_Flanke);
          }
     }
     /* Update previous state */
     MDS_LEFT_LetzterStatus = currentState;
}

#endif


