/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MDS_LEFT_Pin GPIO_PIN_5
#define MDS_LEFT_GPIO_Port GPIOE
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define USER_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define VR1_Pin GPIO_PIN_0
#define VR1_GPIO_Port GPIOC
#define VR2_Pin GPIO_PIN_1
#define VR2_GPIO_Port GPIOC
#define VR3_Pin GPIO_PIN_2
#define VR3_GPIO_Port GPIOA
#define VR4_Pin GPIO_PIN_3
#define VR4_GPIO_Port GPIOA
#define MDS_DOWN_Pin GPIO_PIN_10
#define MDS_DOWN_GPIO_Port GPIOE
#define MDS_DOWN_EXTI_IRQn EXTI15_10_IRQn
#define MDS_UP_Pin GPIO_PIN_15
#define MDS_UP_GPIO_Port GPIOE
#define MDS_UP_EXTI_IRQn EXTI15_10_IRQn
#define DISPLAY_CS_Pin GPIO_PIN_8
#define DISPLAY_CS_GPIO_Port GPIOD
#define DISPLAY_RESET_Pin GPIO_PIN_9
#define DISPLAY_RESET_GPIO_Port GPIOD
#define DISPLAY_DC_Pin GPIO_PIN_10
#define DISPLAY_DC_GPIO_Port GPIOD
#define MDS_BUTTON_Pin GPIO_PIN_14
#define MDS_BUTTON_GPIO_Port GPIOD
#define MDS_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define LED_GREEN_Pin GPIO_PIN_3
#define LED_GREEN_GPIO_Port GPIOD
#define LED_YELLOW_Pin GPIO_PIN_4
#define LED_YELLOW_GPIO_Port GPIOD
#define LED_RED_Pin GPIO_PIN_5
#define LED_RED_GPIO_Port GPIOD
#define LEDM_CS_Pin GPIO_PIN_6
#define LEDM_CS_GPIO_Port GPIOD
#define MDS_RIGHT_Pin GPIO_PIN_5
#define MDS_RIGHT_GPIO_Port GPIOB
#define MDS_RIGHT_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
