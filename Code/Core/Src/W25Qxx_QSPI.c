/*
 * W25Qxx_QSPI.c
 *
 *  Created on: Feb 22, 2025
 *      Author: simim
 */

/*
 * Datasheet: https://www.pjrc.com/teensy/W25Q128FV.pdf
 */

#include "W25Qxx_QSPI.h"

extern OSPI_HandleTypeDef hospi1;

/*
* - `W25Q128_PAGE_SIZE`: Die Größe einer Seite im Speicher, die 256 Bytes beträgt.
* - `W25Q128_SECTOR_SIZE`: Die Größe eines Sektors im Speicher, die 4096 Bytes beträgt.
*/
#define W25Q128_PAGE_SIZE     256
#define W25Q128_SECTOR_SIZE   4096



/**
 * @brief Initialisiert den W25Qxx-Speicherchip.
 *
 * Diese Funktion aktiviert den Quad-Modus des W25Qxx-Speicherchips,
 * um die Datenübertragung über vier Leitungen zu ermöglichen.
 *
 * @return Gibt immer 1 zurück, um den erfolgreichen Abschluss der Initialisierung anzuzeigen.
 *
 * @note Diese Funktion muss vor anderen Speicheroperationen aufgerufen werden,
 *       um sicherzustellen, dass der Quad-Modus aktiviert ist.
 */
uint8_t W25Qxx_begin(){
	W25Qxx_EnableQuadMode();
	return 1;
}



/**
 * @brief Aktiviert den Schreibzugriff für den W25Qxx-Speicherchip.
 *
 * Diese Funktion sendet den Write Enable (WREN) Befehl an den Flash-Speicher,
 * welcher vor Schreib- und Löschoperationen aufgerufen werden muss.
 * Der Write Enable Latch (WEL) wird im Status-Register gesetzt, um
 * Schreibvorgänge zu ermöglichen.
 *
 * @note Nach einer Schreib- oder Löschoperation wird der WEL automatisch zurückgesetzt.
 *       Daher muss diese Funktion vor jedem Schreib- oder Löschvorgang erneut aufgerufen werden.
 *
 * @see W25Qxx_PageProgram(), W25Qxx_EraseSector(), W25Qxx_ChipErase()
 */
void W25Qxx_WriteEnable(){
	OSPI_RegularCmdTypeDef cmd = {0};
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x06;                  // Write Enable command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
	cmd.DataMode = HAL_OSPI_DATA_NONE;
	HAL_OSPI_Command(&hospi1, &cmd, 100);
}



/**
 * @brief Löscht einen Sektor des W25Qxx-Flash-Speicherchips.
 *
 * Diese Funktion löscht einen kompletten Sektor (4KB) des Flash-Speichers an der
 * angegebenen Adresse. Vor dem Aufruf dieser Funktion muss W25Qxx_WriteEnable()
 * aufgerufen werden, um den Schreibzugriff zu aktivieren.
 *
 * @param address Die 24-Bit Adresse des zu löschenden Sektors. Die Adresse sollte
 *                an einer 4KB-Sektorgrenze ausgerichtet sein (vielfaches von 4096).
 *
 * @note Diese Funktion blockiert, bis der Löschvorgang abgeschlossen ist, was
 *       typischerweise 60-300ms dauern kann. Es wird automatisch gewartet, bis
 *       der Löschvorgang abgeschlossen ist.
 *
 * @see W25Qxx_WriteEnable(), W25Qxx_ChipErase(), W25Q128_SECTOR_SIZE
 */
void W25Qxx_EraseSector(uint32_t address){
	OSPI_RegularCmdTypeDef cmd = {0};
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x20;                  // Sector Erase command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.Address = address;                   // 24-bit address
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.DataMode = HAL_OSPI_DATA_NONE;
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	W25Qxx_WaitForWriteComplete();            // Wait for erase to complete
}

/**
 * @brief Programmiert eine Seite im W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion schreibt Daten in eine Seite des Flash-Speichers an der
 * angegebenen Adresse. Vor dem Aufruf dieser Funktion muss W25Qxx_WriteEnable()
 * aufgerufen werden, um den Schreibzugriff zu aktivieren.
 *
 * @param address Die 24-Bit Adresse, an der die Daten geschrieben werden sollen.
 * @param data Zeiger auf den Puffer mit den zu schreibenden Daten.
 * @param size Die Anzahl der zu schreibenden Bytes (maximal 256 Bytes, eine Seite).
 *
 * @note Eine Seite im W25Q128 ist auf 256 Bytes begrenzt. Wenn die Adresse plus die
 *       Datengröße über eine Seitengrenze hinausgeht, wird der Überlauf am Seitenanfang
 *       fortgesetzt (Page Wrap-Around). Um größere Datenmengen zu schreiben, verwenden
 *       Sie W25Qxx_WriteData().
 *
 * @see W25Qxx_WriteEnable(), W25Qxx_WriteData(), W25Q128_PAGE_SIZE
 */
void W25Qxx_PageProgram(uint32_t address, uint8_t *data, uint32_t size){
	OSPI_RegularCmdTypeDef cmd = {0};
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x02;                  // Page Program command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.Address = address;                   // 24-bit address
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.DataMode = HAL_OSPI_DATA_1_LINE;
	cmd.NbData = size;                       // Data size (up to 256 bytes)
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	HAL_OSPI_Transmit(&hospi1, data, 100);
	W25Qxx_WaitForWriteComplete();            // Wait for write to complete
}

/**
 * @brief Wartet, bis ein Schreib- oder Löschvorgang im W25Qxx-Flash-Speicherchip abgeschlossen ist.
 *
 * Diese Funktion prüft wiederholt das Status-Register des W25Qxx-Speicherchips,
 * bis der BUSY-Bit (Bit 0) auf 0 gesetzt ist, was anzeigt, dass eine
 * vorherige Schreib- oder Löschoperation abgeschlossen wurde.
 *
 * @note Diese Funktion wird intern von W25Qxx_PageProgram(), W25Qxx_EraseSector(),
 *       W25Qxx_ChipErase() und anderen Funktionen aufgerufen, die Schreib- oder
 *       Löschvorgänge durchführen, um sicherzustellen, dass die Operation
 *       vollständig abgeschlossen ist, bevor weitere Befehle gesendet werden.
 *
 * @see W25Qxx_PageProgram(), W25Qxx_EraseSector(), W25Qxx_ChipErase()
 */
void W25Qxx_WaitForWriteComplete(void){
	OSPI_RegularCmdTypeDef cmd = {0};
	uint8_t status;

	do {
		cmd.Instruction = 0x05;                // Read Status Register command
		cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
		cmd.DataMode = HAL_OSPI_DATA_1_LINE;
		cmd.NbData = 1;
		HAL_OSPI_Command(&hospi1, &cmd, 100);
		HAL_OSPI_Receive(&hospi1, &status, 100);
	} while (status & 0x01);                 // Wait until BUSY bit clears
}


/**
 * @brief Liest Daten vom W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion führt eine Standard-Leseoperation durch, um Daten von der angegebenen
 * Adresse in einen Puffer zu lesen. Dabei wird der normale Lesebefehl (0x03) verwendet,
 * der Daten über eine einzelne Datenleitung überträgt.
 *
 * @param address Die 24-Bit Adresse, ab der gelesen werden soll.
 * @param buffer Zeiger auf den Puffer, in den die gelesenen Daten gespeichert werden.
 * @param size Die Anzahl der zu lesenden Bytes.
 *
 * @note Diese Funktion verwendet den Standard-Lesebefehl mit einer maximalen Taktrate
 *       von etwa 20-50 MHz. Für höhere Lesegeschwindigkeiten sollten W25Qxx_FastReadData()
 *       oder W25Qxx_FastReadQuadOutput() verwendet werden.
 *
 * @see W25Qxx_FastReadData(), W25Qxx_FastReadQuadOutput()
 */
void W25Qxx_ReadData(uint32_t address, uint8_t *buffer, uint32_t size){
	OSPI_RegularCmdTypeDef cmd = {0};
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x03;                  // Read Data command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.Address = address;                   // 24-bit address
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.DataMode = HAL_OSPI_DATA_1_LINE;
	cmd.NbData = size;                       // Data size
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	HAL_OSPI_Receive(&hospi1, buffer, 100);
}


/**
 * @brief Liest Daten mit erhöhter Geschwindigkeit vom W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion führt eine schnelle Leseoperation durch, um Daten von der angegebenen
 * Adresse in einen Puffer zu lesen. Dabei wird der Fast-Read-Befehl (0x0B) verwendet,
 * welcher höhere Taktraten als der Standard-Lesebefehl unterstützt, jedoch 8 Dummy-Zyklen
 * benötigt.
 *
 * @param address Die 24-Bit Adresse, ab der gelesen werden soll.
 * @param buffer Zeiger auf den Puffer, in den die gelesenen Daten gespeichert werden.
 * @param size Die Anzahl der zu lesenden Bytes.
 *
 * @note Diese Funktion ermöglicht höhere Lesegeschwindigkeiten als W25Qxx_ReadData(),
 *       arbeitet jedoch noch immer mit einer einzelnen Datenleitung. Für maximale
 *       Geschwindigkeit sollte W25Qxx_FastReadQuadOutput() verwendet werden.
 *
 * @see W25Qxx_ReadData(), W25Qxx_FastReadQuadOutput()
 */
void W25Qxx_FastReadData(uint32_t address, uint8_t *buffer, uint32_t size){
	OSPI_RegularCmdTypeDef cmd = {0};
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x0B;                  // Fast Read command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.Address = address;                   // 24-bit address
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.DataMode = HAL_OSPI_DATA_1_LINE;
	cmd.DummyCycles = 8;                     // Dummy cycles (check datasheet)
	cmd.NbData = size;                       // Data size
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	HAL_OSPI_Receive(&hospi1, buffer, 100);
}


/**
 * @brief Liest Daten mit maximaler Geschwindigkeit über Quad-Output vom W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion führt eine Hochgeschwindigkeits-Leseoperation durch, indem sie den
 * Fast Read Quad Output Befehl (0x6B) verwendet. Während Befehl und Adresse über eine
 * einzelne Leitung gesendet werden, erfolgt der Datenempfang über vier parallele Leitungen,
 * was eine deutlich höhere Übertragungsrate ermöglicht.
 *
 * @param address Die 24-Bit Adresse, ab der gelesen werden soll.
 * @param buffer Zeiger auf den Puffer, in den die gelesenen Daten gespeichert werden.
 * @param size Die Anzahl der zu lesenden Bytes.
 *
 * @note Diese Funktion benötigt 8 Dummy-Zyklen zwischen Adresse und Datenübertragung und
 *       erfordert, dass der Quad-Modus zuvor durch W25Qxx_EnableQuadMode() aktiviert wurde.
 *       Sie bietet die höchste Lesegeschwindigkeit unter allen Lesefunktionen.
 *
 * @see W25Qxx_ReadData(), W25Qxx_FastReadData(), W25Qxx_EnableQuadMode()
 */
void W25Qxx_FastReadQuadOutput(uint32_t address, uint8_t *buffer, uint32_t size){
	OSPI_RegularCmdTypeDef cmd = {0};

	// Configure the Fast Read Quad Output command
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x6B;                  // Fast Read Quad Output command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;  // Send instruction on 1 line
	cmd.Address = address;                   // 24-bit address
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE; // Send address on 1 line
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS; // 24-bit address
	cmd.DataMode = HAL_OSPI_DATA_4_LINES;     // Receive data on 4 lines
	cmd.DummyCycles = 8;                      // Dummy cycles (check datasheet)
	cmd.NbData = size;                        // Data size
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	HAL_OSPI_Receive(&hospi1, buffer, 100);   // Receive data
}


/**
 * @brief Aktiviert den Quad-Modus im W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion aktiviert den Quad-Modus im W25Qxx-Speicherchip, indem sie das
 * QE-Bit (Quad Enable Bit) im Status-Register 2 setzt. Im Quad-Modus kann der Chip
 * Daten über vier parallele Leitungen übertragen, was zu einer erheblich höheren
 * Datenübertragungsrate führt.
 *
 * Die Funktion prüft zunächst, ob das QE-Bit bereits gesetzt ist, und setzt es nur,
 * wenn nötig. Dazu wird:
 * 1. Status-Register 2 gelesen
 * 2. Bei Bedarf Write Enable aktiviert
 * 3. Status-Register 2 mit gesetztem QE-Bit zurückgeschrieben
 * 4. Auf Abschluss des Schreibvorgangs gewartet
 *
 * @note Diese Funktion wird typischerweise von W25Qxx_begin() aufgerufen und muss
 *       aktiviert sein, bevor Funktionen wie W25Qxx_FastReadQuadOutput() verwendet werden.
 *
 * @see W25Qxx_begin(), W25Qxx_FastReadQuadOutput()
 */
void W25Qxx_EnableQuadMode(void) {
  uint8_t status;
  OSPI_RegularCmdTypeDef cmd = {0};

  // Read Status Register 2 (0x35)
  cmd.Instruction = 0x35;
  cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.NbData = 1;
  HAL_OSPI_Command(&hospi1, &cmd, 100);
  HAL_OSPI_Receive(&hospi1, &status, 100);

  if (!(status & 0x02)) {  // Check QE bit (bit 1)
    // Write Enable (0x06)
    cmd.Instruction = 0x06;
    cmd.DataMode = HAL_OSPI_DATA_NONE;
    HAL_OSPI_Command(&hospi1, &cmd, 100);

    // Write Status Register 2 (0x31) with QE bit
    uint8_t new_status = status | 0x02;
    cmd.Instruction = 0x31;
    cmd.DataMode = HAL_OSPI_DATA_1_LINE;
    HAL_OSPI_Command(&hospi1, &cmd, 100);
    HAL_OSPI_Transmit(&hospi1, &new_status, 100);

    W25Qxx_WaitForWriteComplete();  // Wait until QE bit is updated
  }
}
/**
 * @brief Liest die Hersteller- und Geräte-ID des W25Qxx-Flash-Speicherchips aus.
 *
 * Diese Funktion sendet den Read Manufacturer/Device ID Befehl (0x90) an den
 * Flash-Speicherchip und liest die zurückgegebenen zwei Bytes aus, die die
 * Hersteller-ID und die Geräte-ID enthalten. Diese Informationen können verwendet
 * werden, um den genauen Typ des verwendeten Speicherchips zu identifizieren.
 *
 * @param manufacturerID Zeiger auf eine Variable, in der die Hersteller-ID gespeichert wird.
 *                       Der Wert 0xEF steht typischerweise für Winbond.
 * @param deviceID Zeiger auf eine Variable, in der die Geräte-ID gespeichert wird.
 *                 Für W25Q128 lautet die Geräte-ID typischerweise 0x17.
 *
 * @note Diese Funktion kann zur Laufzeit verwendet werden, um die Anwesenheit und den
 *       genauen Typ des angeschlossenen Flash-Speicherchips zu überprüfen.
 *
 * @see W25Qxx_begin()
 */
void W25Qxx_Read_Manu_ID(uint8_t *manufacturerID, uint8_t *deviceID){

	OSPI_RegularCmdTypeDef cmd = {0};
	uint8_t data[2];  // Buffer to store both bytes


	// Configure the Read Manufacturer/Device ID command
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.Instruction = 0x90;                  // Read Manufacturer/Device ID command
	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;  // Send instruction on 1 line
	cmd.Address = 0x000000;                  // 24-bit address (usually 0x000000)
	cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE; // Send address on 1 line
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS; // 24-bit address
	cmd.DataMode = HAL_OSPI_DATA_1_LINE;     // Receive data on 1 line
	cmd.DummyCycles = 0;                     // No dummy cycles
	cmd.NbData = 2;                          // Read 2 bytes (Manufacturer ID and Device ID)



	// Send the command and receive the data
	HAL_OSPI_Command(&hospi1, &cmd, 100);
	HAL_OSPI_Receive(&hospi1, data, 100);  // Read both bytes at once

	*manufacturerID = data[0];  // First byte: Manufacturer ID
	*deviceID = data[1];        // Second byte: Device ID
}

/**
 * @brief Schreibt Daten beliebiger Größe in den W25Qxx-Flash-Speicherchip.
 *
 * Diese Funktion ermöglicht das Schreiben von Datenblöcken, die größer als eine
 * einzelne Seite (256 Bytes) sind, indem sie die Daten automatisch in seitenweise
 * Schreibvorgänge aufteilt. Die Funktion beachtet die Seitengrenzen des Flash-Speichers
 * und führt mehrere Schreiboperationen durch, um alle Daten zu speichern.
 *
 * @param address Die 24-Bit Startadresse, an die geschrieben werden soll.
 * @param data Zeiger auf den Puffer mit den zu schreibenden Daten.
 * @param size Die Gesamtanzahl der zu schreibenden Bytes.
 *
 * @note Diese Funktion:
 *       1. Teilt die Daten in passende Seiten-Chunks auf
 *       2. Führt für jeden Chunk einen Write-Enable-Befehl aus
 *       3. Programmiert die einzelnen Seiten nacheinander
 *       4. Wartet nach jeder Seite auf den Abschluss des Schreibvorgangs
 *
 * @see W25Qxx_WriteEnable(), W25Qxx_PageProgram(), W25Qxx_WaitForWriteComplete(),
 *      W25Q128_PAGE_SIZE
 */
void W25Qxx_WriteData(int32_t address, uint8_t *data, uint32_t size){

	uint32_t currentAddress = address;
	uint32_t remainingBytes = size;
	uint32_t bytesToWrite;

	while (remainingBytes > 0) {
		// Calculate the number of bytes we can write in the current page
		uint32_t pageOffset = currentAddress % W25Q128_PAGE_SIZE;
		uint32_t spaceInPage = W25Q128_PAGE_SIZE - pageOffset;
		bytesToWrite = (remainingBytes > spaceInPage) ? spaceInPage : remainingBytes;

		// Enable write operations
		W25Qxx_WriteEnable();

		// Write the page
		W25Qxx_PageProgram(currentAddress, data, bytesToWrite);

		// Wait until the write operation is complete
		W25Qxx_WaitForWriteComplete();

		// Move to the next chunk
		currentAddress += bytesToWrite;
		data += bytesToWrite;
		remainingBytes -= bytesToWrite;
	}
}

/**
 * @brief Löscht den gesamten Inhalt des W25Qxx-Flash-Speicherchips.
 *
 * Diese Funktion führt einen vollständigen Chip-Erase-Vorgang durch, der alle Daten
 * im Flash-Speicherchip löscht und auf 0xFF zurücksetzt. Der Vorgang umfasst drei Hauptschritte:
 * 1. Aktivierung des Schreibzugriffs durch W25Qxx_WriteEnable()
 * 2. Senden des Chip-Erase-Befehls (0xC7)
 * 3. Warten auf den Abschluss des Löschvorgangs
 *
 * @note Der Chip-Erase-Vorgang kann je nach Speicherkapazität mehrere Sekunden bis
 *       Minuten dauern. Bei W25Q128 (16 MB) kann dies typischerweise 15-30 Sekunden
 *       in Anspruch nehmen. Die Funktion blockiert, bis der Löschvorgang vollständig
 *       abgeschlossen ist.
 *
 * @warning Diese Funktion löscht ALLE Daten im Speicherchip ohne Möglichkeit zur
 *          Wiederherstellung. Sie sollte mit Vorsicht verwendet werden.
 *
 * @see W25Qxx_WriteEnable(), W25Qxx_WaitForWriteComplete(), W25Qxx_EraseSector()
 */
void W25Qxx_ChipErase(void) {
  OSPI_RegularCmdTypeDef cmd = {0};

  // Step 1: Enable Write Operations
  W25Qxx_WriteEnable();

  // Step 2: Send Chip Erase Command
  cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd.Instruction = 0xC7;                  // Chip Erase command (0xC7 or 0x60)
  cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  HAL_OSPI_Command(&hospi1, &cmd, 100);

  // Step 3: Wait for Erase to Complete
  W25Qxx_WaitForWriteComplete();
}



