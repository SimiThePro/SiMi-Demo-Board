/*
 * ILI9341.h
 *
 *  Created on: Sep 15, 2024
 *      Author: simim
 */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#include "main.h"
#include "Fonts/gfxfont.h"

/* --------------------------------- Konstanten --------------------------------- */
extern uint16_t ILI9341_WIDTH;
extern uint16_t ILI9341_HEIGHT;

#define BURST_MAX_SIZE 100

/* --------------------------------- Farben --------------------------------- */
#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

/* --------------------------------- Screen Orientation --------------------------------- */
#define SCREEN_VERTICAL_1    0
#define SCREEN_HORIZONTAL_1  1
#define SCREEN_VERTICAL_2    2
#define SCREEN_HORIZONTAL_2  3

// #define USE_JPEG_ENCODING

/**
 * @brief Orientation modes for ILI9341
 */
typedef enum
{
    ILI9341_PORTRAIT = 0,           // 0 degree rotation
    ILI9341_LANDSCAPE = 1,          // 90 degree rotation
    ILI9341_PORTRAIT_INVERTED = 2,  // 180 degree rotation
    ILI9341_LANDSCAPE_INVERTED = 3, // 270 degree rotation
    TEST = 4,
    ILI9341_PORTRAIT_TRUE = 5
} ILI9341_Orientation;

extern ILI9341_t3_font_t *font;

/* --------------------------------- Initialization --------------------------------- */
void ILI9341_begin(SPI_HandleTypeDef *DISPLAY_SPI, GPIO_TypeDef *CS_Port, uint16_t CS_Pin, 
                  GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *Reset_Port, uint16_t Reset_Pin);

/* --------------------------------- Low-level communication --------------------------------- */
HAL_StatusTypeDef ILI9341_SendCommand(uint8_t cmd);
HAL_StatusTypeDef ILI9341_SendCommandWithParam_8Bit(uint8_t cmd, uint8_t *Params, uint8_t pSize);
HAL_StatusTypeDef ILI9341_SendCommandWithParam_16Bit(uint8_t cmd, uint16_t *Params, uint8_t pSize);
HAL_StatusTypeDef ILI9341_SendCommandAndReceive(uint8_t cmd, uint8_t *dataOut, uint8_t pSize);
HAL_StatusTypeDef ILI9341_SendData(uint8_t *Data, uint32_t pSize);
uint8_t ILI9341_ReceiveByte();
HAL_StatusTypeDef ILI9341_ReceiveData(uint8_t *dataOut, uint8_t pSize);

/* --------------------------------- Display control commands --------------------------------- */
void ILI9341_DisplayOn();
void ILI9341_DisplayOff();
void ILI9341_ColumnAddressSet(uint16_t SC, uint16_t EC);
void ILI9341_RowAddressSet(uint16_t SC, uint16_t EC);
void ILI9341_SetAddress(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void ILI9341_MemoryWrite(uint8_t Red, uint8_t Green, uint8_t Blue);
void ILI9341_MemoryWriteRaw(uint8_t *data, uint32_t size);
void ILI9341_DrawColourBurst(uint16_t Colour, uint32_t Size);
void ILI9341_SetRotation(uint8_t Rotation);
void ILI9341_SetOrientation(ILI9341_Orientation orientation);

/* --------------------------------- Basic drawing functions --------------------------------- */
void ILI9341_FillScreen(uint16_t Colour);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_DrawPixelRGB(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

/* --------------------------------- Shapes drawing --------------------------------- */
void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void ILI9341_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ILI9341_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void ILI9341_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void ILI9341_DrawCircleOutline(uint16_t x, uint16_t y, uint8_t r, uint16_t color);
void ILI9341_DrawCircle(uint16_t x, uint16_t y, uint8_t r, uint16_t color);
void ILI9341_DrawFilledCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
void ILI9341_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void ILI9341_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void ILI9341_DrawFilledRoundedRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color);
void ILI9341_DrawRoundedRectWithBorder(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, 
                                      uint16_t fillColor, uint16_t borderColor, uint16_t borderSize);
void ILI9341_DrawBorder(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t borderSize, uint16_t color);

/* --------------------------------- Text and font functions --------------------------------- */
void ILI9341_DrawChar(char Character, uint16_t X, uint16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour);

void ILI9341_DrawText(const char *Text, uint16_t X, uint16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour);

/* --------------------------------- Image drawing functions --------------------------------- */
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image);
void DisplayImageArray(const uint16_t *imageData, uint16_t width, uint16_t height);
void ILI9341_DrawBinaryFile(const char *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height);



#endif /* INC_ILI9341_H_ */
