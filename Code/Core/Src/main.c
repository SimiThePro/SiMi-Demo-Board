/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "fdcan.h"
#include "i2c.h"
#include "usart.h"
#include "memorymap.h"
#include "octospi.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "WS2812.h"
#include "AHT20.h"
#include "ILI9341.h"
#include "LED.h"
#include "LED_Matrix.h"
#include "Realtime.h"
#include "UserInput.h"
#include "SSD1306.h"
#include "Fonts/ssd1306_fonts.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t lastUpdateTime = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


  uint8_t rx_data;
  HAL_I2C_Mem_Read(&hi2c1, 0xA1, 0x02, 1, &rx_data, 1, 100); // Lese von Register 0x02

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_LPUART1_UART_Init();
  MX_UART7_Init();
  MX_OCTOSPI1_Init();
  MX_SDMMC1_SD_Init();
  MX_SPI1_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_TIM6_Init();
  MX_FATFS_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */


  LED_Matrix_setup();
  LED_Matrix_reset();
  uint8_t heart[8]={
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000
  };

  for (int i = 0; i < 8; i++){
    LED_Matrix_draw_row(i,heart[i]);
  }

  LED_Matrix_set_intensity(2);



  ssd1306_Init();

  ssd1306_Fill(White);

  ssd1306_SetCursor(25,0);
  ssd1306_WriteString("Demo Programm", Font_6x8, BLACK);
  ssd1306_UpdateScreen();



  ILI9341_begin(&hspi1, DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, DISPLAY_DC_GPIO_Port, DISPLAY_DC_Pin, DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin);

  ILI9341_DisplayOn();

  ILI9341_SetOrientation(TEST);

  ILI9341_FillScreen(WHITE);

  ILI9341_DrawText("DEMO PROGRAMM", 50, 50, BLACK, 3,WHITE);


  ILI9341_DrawBinaryFile("SiMi_Logo_TFT.bin", 30, 120, 100, 79);
  ILI9341_DrawBinaryFile("TFO_TFT.bin", 180, 120, 100, 79);

  // Herz auf SSD1306 zeichnen
    ssd1306_SetCursor(80, 40);
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (heart[y] & (1 << (7 - x))) {
          ssd1306_DrawPixel(80 + x*2, 40 + y*2, BLACK);
          ssd1306_DrawPixel(80 + x*2 + 1, 40 + y*2, BLACK);
          ssd1306_DrawPixel(80 + x*2, 40 + y*2 + 1, BLACK);
          ssd1306_DrawPixel(80 + x*2 + 1, 40 + y*2 + 1, BLACK);
        }
      }
    }
    ssd1306_UpdateScreen();


  Realtime_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#ifdef USE_POLLING
    PollingUserInput();
#endif
#ifdef USE_INTERRUPT
    HandleMDSLeft();
#endif


    HandlePendingUserInput();


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  if (MDS_LEFT_Flanke == 1) {
    printf("MDS_LEFT Flanke erkannt\n");
    ILI9341_fillRect(0,0,320,40,WHITE);
    ILI9341_DrawText("LEFT",130,0,BLACK,3,WHITE);
  }
  else if (MDS_RIGHT_Flanke == 1) {
    printf("MDS_RIGHT_Flanke erkannt\n");
    ILI9341_fillRect(0,0,320,40,WHITE);
    ILI9341_DrawText("RIGHT",130,0,BLACK,3,WHITE);
  }
  else if (MDS_UP_Flanke == 1) {
    printf("MDS_UP_Flanke erkannt\n");
    ILI9341_fillRect(0,0,320,40,WHITE);
    ILI9341_DrawText("UP",130,0,BLACK,3,WHITE);
  }
  else if (MDS_DOWN_Flanke == 1) {
    printf("MDS_DOWN_Flanke erkannt\n");
    ILI9341_fillRect(0,0,320,40,WHITE);
    ILI9341_DrawText("DOWN",130,0,BLACK,3,WHITE);
  }
    else if (MDS_BUTTON_Flanke == 1){
      printf("MDS_BUTTON_Flanke erkannt\n");
      ILI9341_fillRect(0,0,320,40,WHITE);
      ILI9341_DrawText("BUTTON",130,0,BLACK,3,WHITE);
    }
    else if (USER_BUTTON_Flanke == 1) {
      printf("USER_BUTTON_Flanke erkannt\n");
      ILI9341_fillRect(0,0,320,40,WHITE);
    }


    UpdatePotiValues();

  WS2812_SetLED(Poti1Value*(1.0/256.0),Poti2Value*(1.0/256.0),Poti3Value*(1.0/256.0));
  WS2812_SetBrightness(Poti4Value*(45.0/65536.0));
  WS2812_Send();
  HAL_Delay(1);


    if (HAL_GetTick() - lastUpdateTime >= 1000) {  // Alle 1000 ms aktualisieren
      float temp = 0, hum = 0;
      AHT20_Read(&temp, &hum);
        char tempStr[16], humStr[16];
        sprintf(tempStr, "Temp: %.1f C", temp);
        sprintf(humStr, "Hum: %.1f %%", hum);

        // Bereich löschen und neu beschreiben
        ssd1306_Fill(White);
        ssd1306_SetCursor(25, 0);
        ssd1306_WriteString("Demo Programm", Font_6x8, BLACK);
        ssd1306_SetCursor(10, 20);
        ssd1306_WriteString(tempStr, Font_6x8, BLACK);
        ssd1306_SetCursor(10, 30);
        ssd1306_WriteString(humStr, Font_6x8, BLACK);
        ssd1306_UpdateScreen();

      lastUpdateTime = HAL_GetTick();
    }

    ResetFlanken();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*AXI clock gating */
  RCC->CKGAENR = 0xFFFFFFFF;

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//TODO DO CODE EINI TIAN
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart7, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    //Alle Funktionen, welche den Interrupt abbekommen wollen werden hier drinnen platziert.
    //Wenn man nicht weiß, was man tut, hier nichts verändern

#ifdef USE_INTERRUPT
    UserInput_Interrupt(GPIO_Pin);
#endif

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

#ifdef DEBOUNCE_WITH_TIMER
  if (htim->Instance == TIM6) {
    HandleDebouncedUserInput();

    // Stoppe den Timer
    HAL_TIM_Base_Stop(&htim6);

    // Setze debounce flag zurück
    debounce_in_progress = 0;

    //Setzte timer update flag zurück
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
  }
#endif


  if (htim->Instance == TIM7) {
    // Timer 7 Callback
    Realtime_Loop();
  }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
