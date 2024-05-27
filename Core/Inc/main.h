/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define API_GET_CODE_IOTVISION				"http://App.IoTVision.vn/api/DCa_DuLieuGuiXuongBoard?CheDo=1&key="
#define API_POST_CODE_IOTVISION				"http://app.iotvision.vn/api/DCa_DuLieuGuiXuongBoard"

#define API_POST_DATA_IOTVISION 			"http://App.IoTVision.vn/api/DCa_DuLieu"


#define API_GET_TEST 			"http://app.iotvision.vn/api/ThoiGian"
#define API_POST_DATA_DEOCA 	"https://k8s.deoca.vn/beta/v2/lora/test"


#define RUN_SIM
#define RUN_GPS
#define RUN_LigoSENSOR

#ifdef RUN_LigoSENSOR
#include "../libraries/LIGOSensor/uart.h"
#endif

#ifdef RUN_SIM
#include "../libraries/sim7600/sim7600.h"
#endif

#ifdef RUN_GPS
#include "../libraries/TAU1201/TAU1201.h"
#endif

#include "../libraries/DS3231/DS3231.h"

#include "../libraries/24LCxx/24LCxx.h"

#include "timer_flag.h"

#define EEPROM_I2C_ADDRESS              0x50
#define EEPROM_SIZE                     256

typedef struct{
	int CodeGet;

	int HTTPGetCode;
	int HTTPPostCode;
	char HTTPGetResponse[256];
	char HTTPPostResponse[256];
}server_t;


typedef struct{
	uint8_t update_status;
//	Board's ID
	char ID[15];

//	Thông tin tài xế
	char DriverID[20];

//	3 ngưỡng giới hạn tốc độ
	uint16_t speed_limit_1;
	uint16_t speed_limit_2;
	uint16_t speed_limit_3;


}board_parameter_t;

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
#define ACC_Pin GPIO_PIN_2
#define ACC_GPIO_Port GPIOC
#define ACC_EXTI_IRQn EXTI2_IRQn
#define SIM_RST_Pin GPIO_PIN_5
#define SIM_RST_GPIO_Port GPIOC
#define SIM_STATUS_Pin GPIO_PIN_0
#define SIM_STATUS_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_1
#define BUZZER_GPIO_Port GPIOB
#define SIM_PWR_Pin GPIO_PIN_15
#define SIM_PWR_GPIO_Port GPIOA
#define LED_SENSOR_Pin GPIO_PIN_4
#define LED_SENSOR_GPIO_Port GPIOD
#define LED_DRIVER_Pin GPIO_PIN_5
#define LED_DRIVER_GPIO_Port GPIOD
#define LED_SERVER_Pin GPIO_PIN_6
#define LED_SERVER_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
