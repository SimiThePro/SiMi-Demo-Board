/*
 * SDCard.c
 *
 *  Created on: Mar 1, 2025
 *      Author: simim
 */

#include "SDCard.h"
#include "main.h"
#include "fatfs.h"
#include <stdio.h>
#include "ILI9341.h"
#include <string.h>
extern SD_HandleTypeDef hsd1;
char TxBuffer[250];



/**
* @brief Führt einen Test der SD-Karten-Funktionalität durch.
*
* Diese Funktion demonstriert grundlegende SD-Karten-Operationen:
* - Mounten der SD-Karte
* - Ermitteln der SD-Kartengröße und des freien Speicherplatzes
* - Erstellen und Schreiben einer Textdatei
* - Lesen der geschriebenen Datei
* - Aktualisieren einer bestehenden Datei
* - Löschen der Datei
* - Unmounten der SD-Karte
*/
  void SDIO_SDCard_Test(void)
  {
    FATFS FatFs;          // FAT-Dateisystem-Objekt
    FIL Fil;              // Dateiobjekt
    FRESULT FR_Status;    // Ergebnisstatus der FatFs-Funktionen
    FATFS *FS_Ptr;        // Zeiger auf das FAT-Dateisystem
    UINT RWC, WWC;        // Lese-/Schreibzähler
    DWORD FreeClusters;   // Anzahl der freien Cluster
    uint32_t TotalSize, FreeSpace; // Gesamtspeicher und freier Speicherplatz
    char RW_Buffer[200];  // Puffer für Lese-/Schreiboperationen

    do
    {
      //------------------[ Mounten der SD-Karte ]--------------------
      FR_Status = f_mount(&FatFs, (TCHAR const*)SDPath, 1);
      if (FR_Status != FR_OK)
      {
        // Fehler beim Mounten der SD-Karte
        sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
        printf(TxBuffer);
        break;
      }
      sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");
      printf(TxBuffer);

      //------------------[ Ermitteln der SD-Kartengröße und des freien Speicherplatzes ]--------------------
      f_getfree("", &FreeClusters, &FS_Ptr);
      TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5); // Gesamtspeicher in Bytes
      FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);          // Freier Speicherplatz in Bytes
      sprintf(TxBuffer, "Total SD Card Size: %lu Bytes\r\n", TotalSize);
      printf(TxBuffer);
      sprintf(TxBuffer, "Free SD Card Space: %lu Bytes\r\n\n", FreeSpace);
      printf(TxBuffer);

      //------------------[ Erstellen und Schreiben einer Textdatei ]--------------------
      // Öffnen oder Erstellen der Datei
      FR_Status = f_open(&Fil, "MyTextFile.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
      if(FR_Status != FR_OK)
      {
        // Fehler beim Erstellen/Öffnen der Datei
        sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
        printf(TxBuffer);
        break;
      }
      sprintf(TxBuffer, "Text File Created & Opened! Writing Data To The Text File..\r\n\n");
      printf(TxBuffer);

      // Schreiben von Daten in die Datei mit f_puts()
      f_puts("Hello! From STM32 To SD Card Over SDMMC, Using f_puts()\n", &Fil);

      // Schreiben von Daten in die Datei mit f_write()
      strcpy(RW_Buffer, "Hello! From STM32 To SD Card Over SDMMC, Using f_write()\r\n");
      f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC);

      // Schließen der Datei
      f_close(&Fil);

      //------------------[ Lesen der Textdatei ]--------------------
      // Öffnen der Datei zum Lesen
      FR_Status = f_open(&Fil, "MyTextFile.txt", FA_READ);
      if(FR_Status != FR_OK)
      {
        // Fehler beim Öffnen der Datei
        sprintf(TxBuffer, "Error! While Opening (MyTextFile.txt) File For Read.. \r\n");
        printf(TxBuffer);
        break;
      }

      // Lesen der Datei mit f_gets()
      f_gets(RW_Buffer, sizeof(RW_Buffer), &Fil);
      sprintf(TxBuffer, "Data Read From (MyTextFile.txt) Using f_gets():%s", RW_Buffer);
      printf(TxBuffer);

      // Lesen der Datei mit f_read()
      f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
      sprintf(TxBuffer, "Data Read From (MyTextFile.txt) Using f_read():%s", RW_Buffer);
      printf(TxBuffer);

      // Schließen der Datei
      f_close(&Fil);
      sprintf(TxBuffer, "File Closed! \r\n\n");
      printf(TxBuffer);

      //------------------[ Aktualisieren der Datei und erneutes Lesen ]--------------------
      // Öffnen der Datei zum Aktualisieren
      FR_Status = f_open(&Fil, "MyTextFile.txt", FA_OPEN_EXISTING | FA_WRITE);
      FR_Status = f_lseek(&Fil, f_size(&Fil)); // Verschieben des Dateizeigers ans Ende
      if(FR_Status != FR_OK)
      {
        // Fehler beim Öffnen der Datei
        sprintf(TxBuffer, "Error! While Opening (MyTextFile.txt) File For Update.. \r\n");
        printf(TxBuffer);
        break;
      }

      // Hinzufügen einer neuen Zeile
      FR_Status = f_puts("This New Line Was Added During File Update!\r\n", &Fil);
      f_close(&Fil);

      // Puffer leeren
      memset(RW_Buffer,'\0',sizeof(RW_Buffer));

      // Lesen der Datei nach der Aktualisierung
      FR_Status = f_open(&Fil, "MyTextFile.txt", FA_READ);
      f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
      sprintf(TxBuffer, "Data Read From (MyTextFile.txt) After Update:\r\n%s", RW_Buffer);
      printf(TxBuffer);
      f_close(&Fil);

      //------------------[ Löschen der Datei ]--------------------
      FR_Status = f_unlink("MyTextFile.txt");
      if (FR_Status != FR_OK)
      {
          // Fehler beim Löschen der Datei
          sprintf(TxBuffer, "Error! While Deleting The (MyTextFile.txt) File.. \r\n");
          printf(TxBuffer);
      }

    } while(0);

	//------------------[ Unmounten der SD-Karte ]--------------------
	FR_Status = f_mount(NULL, "", 0);
	if (FR_Status != FR_OK)
	{
	    // Fehler beim Unmounten der SD-Karte
	    sprintf(TxBuffer, "\r\nError! While Un-mounting SD Card, Error Code: (%i)\r\n", FR_Status);
	    printf(TxBuffer);
	}
	else
	{
	    sprintf(TxBuffer, "\r\nSD Card Un-mounted Successfully! \r\n");
	    printf(TxBuffer);
	}
  }

/**
 * @brief Mountet die SD-Karte.
 *
 * Diese Funktion versucht, die SD-Karte zu mounten, und gibt den Erfolg oder
 * Misserfolg zurück. Sie gibt entsprechende Statusmeldungen über die serielle
 * Schnittstelle aus.
 *
 * @return uint8_t
 *         - 1: Erfolgreiches Mounten der SD-Karte
 *         - 0: Fehler beim Mounten der SD-Karte
 */
uint8_t mountSD() {
    FATFS FatFs;          // FAT-Dateisystem-Objekt
    FIL Fil;              // Dateiobjekt (nicht verwendet in dieser Funktion)
    FRESULT FR_Status;    // Ergebnisstatus der FatFs-Funktionen
    FATFS *FS_Ptr;        // Zeiger auf das FAT-Dateisystem (nicht verwendet in dieser Funktion)

    // Versuche, die SD-Karte zu mounten
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK) {
        // Fehler beim Mounten der SD-Karte
        sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
        printf(TxBuffer);
        return 0; // Rückgabe 0 bei Fehler
    }

    // Erfolgreiches Mounten der SD-Karte
    sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");
    printf(TxBuffer);
    return 1; // Rückgabe 1 bei Erfolg
}

/**
  * @brief Unmountet die SD-Karte.
  *
  * Diese Funktion versucht, die SD-Karte sicher zu unmounten und gibt den Erfolg
  * oder Misserfolg zurück. Sie gibt entsprechende Statusmeldungen über die serielle
  * Schnittstelle aus.
  *
  * @return uint8_t
  *         - 1: Erfolgreiches Unmounten der SD-Karte
  *         - 0: Fehler beim Unmounten der SD-Karte
  */
 uint8_t unmountSD() {
     FATFS FatFs;          // FAT-Dateisystem-Objekt (nicht verwendet in dieser Funktion)
     FIL Fil;              // Dateiobjekt (nicht verwendet in dieser Funktion)
     FRESULT FR_Status;    // Ergebnisstatus der FatFs-Funktionen
     FATFS *FS_Ptr;        // Zeiger auf das FAT-Dateisystem (nicht verwendet in dieser Funktion)

     // Versuche, die SD-Karte zu unmounten
     FR_Status = f_mount(NULL, "", 0);
     if (FR_Status != FR_OK) {
         // Fehler beim Unmounten der SD-Karte
         sprintf(TxBuffer, "\r\nError! While Un-mounting SD Card, Error Code: (%i)\r\n", FR_Status);
         printf(TxBuffer);
         return 0; // Rückgabe 0 bei Fehler
     } else {
         // Erfolgreiches Unmounten der SD-Karte
         sprintf(TxBuffer, "\r\nSD Card Un-mounted Successfully! \r\n");
         printf(TxBuffer);
     }
     return 1; // Rückgabe 1 bei Erfolg
 }

