/*
 * W25Qxx_QSPI.h
 *
 * Treiber für Winbond W25Qxx Flash-Speicherchips über QSPI-Schnittstelle.
 * Unterstützt Standard-Lese/Schreib-Operationen sowie Quad-Modus für schnellere Datenübertragung.
 *
 * Diese Bibliothek ermöglicht:
 * - Lesen/Schreiben von Daten im Byte-Format
 * - Seiten-basierte Programmierung (256 Bytes)
 * - Sektor-Löschungen (4KB)
 * - Chip-Löschung
 * - Quad-SPI Unterstützung für schnellere Übertragungsraten
 * 
 * Angeschlossener Chip sollte mit STM32 QSPI-Interface verbunden sein.
 * 
 *  Created on: Feb 22, 2025
 *      Author: simim
 */

#ifndef INC_W25QXX_QSPI_H_
#define INC_W25QXX_QSPI_H_

#include "main.h"

/**
 * @brief  Initialisiert den W25Qxx Flash-Speicherchip
 * @retval Status: 0 bei Fehler, 1 bei erfolgreicher Initialisierung
 */
uint8_t W25Qxx_begin();

/**
 * @brief  Wartet, bis eine Schreib- oder Löschoperation abgeschlossen ist
 * @note   Diese Funktion blockiert, bis der Chip bereit für neue Befehle ist
 */
void W25Qxx_WaitForWriteComplete(void);

/**
 * @brief  Aktiviert den Schreibzugriff auf den Flash-Speicher
 * @note   Muss vor jeder Schreib- oder Löschoperation aufgerufen werden
 */
void W25Qxx_WriteEnable();

/**
 * @brief  Löscht einen 4KB Sektor an der angegebenen Adresse
 * @param  address: Startadresse des zu löschenden Sektors
 * @note   Der gesamte 4KB-Sektor wird auf 0xFF zurückgesetzt
 */
void W25Qxx_EraseSector(uint32_t address);

/**
 * @brief  Programmiert eine Seite (bis zu 256 Bytes) im Flash-Speicher
 * @param  address: Zieladresse im Flash (muss Seiten-ausgerichtet sein)
 * @param  data: Zeiger auf die zu schreibenden Daten
 * @param  size: Größe der zu schreibenden Daten (max. 256 Bytes)
 * @note   Die Seite muss vorher gelöscht sein (0xFF)
 */
void W25Qxx_PageProgram(uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief  Liest Daten vom Flash im Standard-Modus
 * @param  address: Quelladresse im Flash
 * @param  buffer: Zielpuffer für die gelesenen Daten
 * @param  size: Anzahl zu lesender Bytes
 */
void W25Qxx_ReadData(uint32_t address, uint8_t *buffer, uint32_t size);

/**
 * @brief  Liest Daten im Fast-Read-Modus (höhere Taktfrequenz)
 * @param  address: Quelladresse im Flash
 * @param  buffer: Zielpuffer für die gelesenen Daten
 * @param  size: Anzahl zu lesender Bytes
 */
void W25Qxx_FastReadData(uint32_t address, uint8_t *buffer, uint32_t size);

/**
 * @brief  Liest die Hersteller- und Geräte-ID des Flash-Chips
 * @param  manufacturerID: Zeiger, in dem die Hersteller-ID gespeichert wird
 * @param  deviceID: Zeiger, in dem die Geräte-ID gespeichert wird
 */
void W25Qxx_Read_Manu_ID(uint8_t *manufacturerID, uint8_t *deviceID);

/**
 * @brief  Liest Daten im Quad-Output-Modus (4 Datenleitungen)
 * @param  address: Quelladresse im Flash
 * @param  buffer: Zielpuffer für die gelesenen Daten
 * @param  size: Anzahl zu lesender Bytes
 * @note   Quad-Modus muss vorher aktiviert sein
 */
void W25Qxx_FastReadQuadOutput(uint32_t address, uint8_t *buffer, uint32_t size);

/**
 * @brief  Aktiviert den Quad-SPI-Modus für schnellere Datenübertragung
 * @note   Nach dem Aufruf können Quad-Funktionen verwendet werden
 */
void W25Qxx_EnableQuadMode(void);

/**
 * @brief  Löscht den gesamten Inhalt des Flash-Chips
 * @note   Diese Operation kann mehrere Sekunden dauern
 */
void W25Qxx_ChipErase(void);

/**
 * @brief  Schreibt Daten beliebiger Größe an eine bestimmte Adresse
 * @param  address: Zieladresse im Flash
 * @param  data: Zeiger auf die zu schreibenden Daten
 * @param  size: Größe der zu schreibenden Daten
 * @note   Diese Funktion kümmert sich um die Aufteilung in Seiten,
 *         automatisches Löschen bei Bedarf und korrekte Adressierung
 */
void W25Qxx_WriteData(int32_t address, uint8_t *data, uint32_t size);

#endif /* INC_W25QXX_QSPI_H_ */
