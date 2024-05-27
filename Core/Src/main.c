/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#ifdef RUN_SIM
sim7600_t sim;
#endif

#ifdef RUN_GPS
TAU1201_t gps;
#endif

flagType _Flag;

DS3231_data_t date_time;

eeprom_24LCxx_t eeprom;
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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t ACC_State;
float Latitude, Longitude, Speed, Satelites;

int ServerConnected;

char debugStr[100];

#ifdef RUN_SIM

#endif

#ifdef RUN_LigoSENSOR

uint8_t data_txCommand[4];
uint8_t data_rxResponse[5];

// declare variable processing UART data
uint8_t rxBuffer[FRAM_SIZE];
uint8_t rx_buff;

int8_t rxIndex = -1;
uint8_t flag_Received = 0;

// declare variable FRAME
FrameData frame ;

// declare variable CHECKSUM
uint8_t Checksum_Response;
uint8_t Checksum_Periodly_Data;

server_t _Server;

board_parameter_t BoardParameter;

#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
#ifdef RUN_LigoSENSOR
	if(huart == &huart1 &&flag_Received == 0){
		// save received byte into rxBuffer
		flag_Received = 1;
		Checksum_Response = data_txCommand_Checksum(data_rxResponse, 4);
		HAL_UART_Receive_IT(&huart1, &rx_buff, 1);
	}
	if(huart == &huart1 && flag_Received == 1 ){
		receive_rx(rx_buff);
		rxBuffer[rxIndex++] = uart_read();
		if(rxIndex == FRAM_SIZE){
			Process_Received_Data(rxBuffer, &frame);
			Checksum_Periodly_Data = data_txCommand_Checksum(rxBuffer, 10);
			rxIndex = 0;
			HAL_UART_Receive_IT(&huart1, &rx_buff, 1);
		}
		else{
			HAL_UART_Receive_IT(&huart1, &rx_buff, 1);
		}
	}
#endif

// GPS Interrupt
#ifdef RUN_GPS
	if(huart == &huart3){
		GPS_UART_CallBack(&gps);
	}
#endif
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
#ifdef RUN_SIM
	SIM_UART_DMA_CallBack(&sim);
#endif
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	ACC_State = HAL_GPIO_ReadPin(ACC_GPIO_Port, ACC_Pin);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//	Interrupt 50ms
	if(htim->Instance == TIM2){
		Flag_CallBack(&_Flag);
	}
//	Interrupt 2S
	if(htim->Instance == TIM3){

	}
}

char DataToPost[200];

char DebugGPSLocation[200];

void PostDataToServer(){
	DS3231_read_time(&date_time);
	sprintf(DataToPost,"{\"ID\": \"%s\",\"S\": \"%f;%f;%f;%d;0;0;%d;32.5;80.6;%d%d;%d:%d:%d %d/%d/%d;790136787923;1\"}", BoardParameter.ID, Longitude, Latitude, Speed, ACC_State, frame.rxRelative_level, sim.Signal, sim.NetTech, date_time.Hour, date_time.Minute, date_time.Second, date_time.Date, date_time.Month, (date_time.Year + 2000));
	if(http_post(&sim, API_POST_DATA_IOTVISION, DataToPost) == SIM7600_OK){
		ServerConnected = 1;
	}
	else{
		ServerConnected = 0;
	}

	if(http_post(&sim, API_POST_DATA_DEOCA, DataToPost) == SIM7600_OK){
		ServerConnected = 1;
	}
	else{
		ServerConnected = 0;
	}
}

void GetCodeFromServer(){
	char urlStr[100];
	sprintf(urlStr, "%s%s", API_GET_CODE_IOTVISION, BoardParameter.ID);

	if(http_get(&sim, urlStr, _Server.HTTPGetResponse, &_Server.HTTPGetCode) == SIM7600_OK){
		if(_Server.HTTPGetCode == 200){
			int foundRes = 0;
			int count = 0;
			for(int i = 0; i <= 256; ++i){
				if(_Server.HTTPGetResponse[i] == '\"') ++count;
				if(count == 7){
					foundRes = i + 1;
					break;
				}
			}

			_Server.CodeGet = 0;
			for(int i = foundRes; i<=256; ++i){
				if(_Server.HTTPGetResponse[i] == ';') break;
				if(_Server.HTTPGetResponse[i] == '\"') break;
				_Server.CodeGet *= 10;
				_Server.CodeGet += _Server.HTTPGetResponse[foundRes] - 48;
			}

			switch (_Server.CodeGet){
				case 3:
	//				[{"ID":"51A94887","S":"3;65M103466"}]
					char oldID[15];
					strcpy(oldID, BoardParameter.ID);

					memset(BoardParameter.ID, 0, sizeof(BoardParameter.ID));
					count = 0;
					for(int i = foundRes + 2; i <= 256; ++i){
						if(_Server.HTTPGetResponse[i] == '\"') break;
						BoardParameter.ID[count] = _Server.HTTPGetResponse[i];
						++count;
					}


					char DataResponseToServer[100];
					sprintf(DataResponseToServer, "{\"ID\":\"%s\",\"S\":\"31\"}", oldID);

					if((http_post(&sim, API_POST_CODE_IOTVISION, DataResponseToServer) == SIM7600_ERROR) || (strlen(BoardParameter.ID) == 0)){
						memset(BoardParameter.ID, 0, sizeof(BoardParameter.ID));
						strcpy(BoardParameter.ID, oldID);
					}
					else{
						EEPROM_write_bytes(&eeprom, 0, (uint8_t *)&BoardParameter, sizeof(BoardParameter));
					}

					break;
				case 4:

					break;
			}

			memset(_Server.HTTPGetResponse, 0, sizeof(_Server.HTTPGetResponse));
		}
	}



	//		sscanf(_Server.HTTPGetResponse, "[{\"ID\":\"51A94887\",\"S\":\"%d\"}]", &_Server.get_code);
	//
	//		if(_Server.get_code == 9){
	//			HAL_Delay(1000);
	//			_Server.get_code = 0;
	//			sprintf(_Server.HTTPGetResponse, "[{\"ID\":\"51A94887\",\"S\":\"%d\"}]", _Server.get_code);
	//			http_post(&sim, API_POST_CODE_IOTVISION, _Server.HTTPGetResponse);
	//		}
	//				http_get(&sim, API_GET_TEST, HTTPGetResponse, &HTTPGetCode);
}

void Bip(){
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);

}

void RunMainProgram(){
// Timer 50ms
#ifdef FLAG_50ms
	if(_Flag.t50ms){

		_Flag.t50ms = 0;
	}
#endif

// Timer 100ms
#ifdef FLAG_100ms
	if(_Flag.t100ms){

		_Flag.t100ms = 0;
	}
#endif

// Timer 250ms
#ifdef FLAG_250ms
	if(_Flag.t250ms){

		_Flag.t250ms = 0;
	}
#endif

// Timer 500ms
#ifdef FLAG_500ms
	if(_Flag.t500ms){
		if(flag_Received == 1){
			HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port, LED_SENSOR_Pin, GPIO_PIN_SET);
		}
		else{
			HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port, LED_SENSOR_Pin, GPIO_PIN_RESET);
		}

		if(ServerConnected == 1){
			HAL_GPIO_WritePin(LED_SERVER_GPIO_Port, LED_SERVER_Pin, GPIO_PIN_SET);
		}
		else{
			HAL_GPIO_WritePin(LED_SERVER_GPIO_Port, LED_SERVER_Pin, GPIO_PIN_RESET);
		}
		_Flag.t500ms = 0;
	}
#endif

// Timer 1s
#ifdef FLAG_1s
	if(_Flag.t1s){

		GetCodeFromServer();

		_Flag.t1s = 0;
	}
#endif

// Timer 1500ms
#ifdef FLAG_1500ms
	if(_Flag.t1500ms){

		_Flag.t1500ms = 0;
	}
#endif

// Timer 2s
#ifdef FLAG_2s
	if(_Flag.t2s){
#ifdef RUN_SIM
//		CheckSimConnect(&sim);
		GetSignal(&sim);
#endif

		Satelites = getSatelitesNum(&gps);
		if((Satelites < 8) && (sim.Signal > 0)){
			GetLBS(&sim);
			Latitude = sim.LBS_Loc.Latitude;
			Longitude = sim.LBS_Loc.Longitude;
			Speed = 0;
		}
		else{
			Latitude = getLatitude(&gps);
			Longitude = getLongitude(&gps);
			Speed = getSpeed(&gps);
		}
		sprintf(DebugGPSLocation,"https://www.google.com/maps/?q=%f,%f\r\n", Latitude, Longitude);

		HAL_UART_Transmit(&huart4, (uint8_t *)DebugGPSLocation, strlen(DebugGPSLocation), 1000);
		_Flag.t2s = 0;
	}
#endif

// Timer 3s
#ifdef FLAG_3s
	if(_Flag.t3s){

		_Flag.t3s = 0;
	}
#endif

// Timer 4s
#ifdef FLAG_4s
	if(_Flag.t4s){

		_Flag.t4s = 0;
	}
#endif

// Timer 5s
#ifdef FLAG_5s
	if(_Flag.t5s){
		//#ifdef RUN_SIM
		//	  http_get(&sim, API_GET_TEST, HTTPGetResponse, &HTTPGetCode);
		//	  HAL_Delay(100);
		//	  http_post(&sim, API_POST_DATA_DEOCA, "{\"ID\":\"B0B21C9ED20C\",\"s\":\"11001111;32.5;80.6;79;20:11:13 06/03/2024\"}");
		//	  HAL_Delay(100);
		//#endif

		_Flag.t5s = 0;
	}
#endif

// Timer 6s
#ifdef FLAG_6s
	if(_Flag.t6s){

		_Flag.t6s = 0;
	}
#endif

// Timer 10s
#ifdef FLAG_10s
	if(_Flag.t10s){

		_Flag.t10s = 0;
	}
#endif

// Timer 15s
#ifdef FLAG_15s
	if(_Flag.t15s){
#ifdef RUN_SIM
//		if(ACC_State == 1){
			PostDataToServer();

			Bip();
//		}
#endif
		_Flag.t15s = 0;
	}
#endif

// Timer 30s
#ifdef FLAG_30s
	if(_Flag.t30s){

		_Flag.t30s = 0;
	}
#endif

// Timer 35s
#ifdef FLAG_35s
	if(_Flag.t35s){

		_Flag.t35s = 0;
	}
#endif

// Timer 15p
#ifdef FLAG_15p
	if(_Flag.t15p){
#ifdef RUN_SIM

		GetLBS(&sim);

		if(sim.LBS_Date.Date != 0) date_time.Date = sim.LBS_Date.Date;
		if(sim.LBS_Date.Day != 0) date_time.Day = sim.LBS_Date.Day;
		if(sim.LBS_Date.Month != 0) date_time.Month = sim.LBS_Date.Month;
		if(sim.LBS_Date.Year != 0) date_time.Year = sim.LBS_Date.Year;

		if(sim.LBS_Date.Hour != 0) date_time.Hour = sim.LBS_Date.Hour;
		if(sim.LBS_Date.Minute != 0) date_time.Minute = sim.LBS_Date.Minute;
		if(sim.LBS_Date.Second != 0) date_time.Second = sim.LBS_Date.Second;

		DS3231_write_time(&date_time);
#endif
		_Flag.t15p = 0;
	}
#endif

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);


  HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port, LED_SENSOR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_DRIVER_GPIO_Port, LED_DRIVER_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_SERVER_GPIO_Port, LED_SERVER_Pin, GPIO_PIN_SET);

  rtc_init(&hi2c1);
  DS3231_read_time(&date_time);

  EEPROM_init(&eeprom, &hi2c1, EEPROM_I2C_ADDRESS, EEPROM_SIZE);

//  BoardParameter.update_status = 1;
//  strcpy(BoardParameter.ID, "51A94887");
//  EEPROM_write_bytes(&eeprom, 0, (uint8_t *)&BoardParameter, sizeof(BoardParameter));
//  clear_EEPROM(&eeprom);

  EEPROM_read_bytes(&eeprom, 0, (uint8_t *)&BoardParameter, sizeof(BoardParameter));

#ifdef RUN_LigoSENSOR
  Command_Format(data_txCommand, data_rxResponse);
#endif


#ifdef RUN_GPS
// Init GPS module
  GPS_Init(&gps, &huart3);
#endif

#ifdef RUN_SIM
// Init SIMCOM module
  SIMInit(&sim, &huart2, &hdma_usart2_rx, SIM_PWR_Pin, SIM_PWR_GPIO_Port, SIM_RST_Pin, SIM_RST_GPIO_Port, SIM_STATUS_Pin, SIM_STATUS_GPIO_Port);
  HAL_Delay(1000);

//  if((sim.SimConnect == 1) && (BoardParameter.update_status == 0)){
//	  strcpy(BoardParameter.ID, sim.PhoneNumber);
//	  EEPROM_write_bytes(&eeprom, 0, (uint8_t *)&BoardParameter, sizeof(BoardParameter));
//  }

//  SendCommand(&sim, "AT+CNUM\r\n", SIM_OK, 1000, 0);
//  GetPhoneNumber(&sim);
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  RunMainProgram();

//	  luu("/Backup/text.txt", "")
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 128;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 49;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SIM_PWR_GPIO_Port, SIM_PWR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_SENSOR_Pin|LED_DRIVER_Pin|LED_SERVER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5
                           PE6 PE7 PE8 PE9
                           PE10 PE11 PE12 PE13
                           PE14 PE15 PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0
                           PC1 PC3 PC4 PC6
                           PC7 PC8 PC9 PC10
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ACC_Pin */
  GPIO_InitStruct.Pin = ACC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ACC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7
                           PA8 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SIM_RST_Pin */
  GPIO_InitStruct.Pin = SIM_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SIM_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SIM_STATUS_Pin */
  GPIO_InitStruct.Pin = SIM_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SIM_STATUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5
                           PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11
                           PD12 PD13 PD14 PD15
                           PD0 PD1 PD2 PD3
                           PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : SIM_PWR_Pin */
  GPIO_InitStruct.Pin = SIM_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SIM_PWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_SENSOR_Pin LED_DRIVER_Pin LED_SERVER_Pin */
  GPIO_InitStruct.Pin = LED_SENSOR_Pin|LED_DRIVER_Pin|LED_SERVER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
