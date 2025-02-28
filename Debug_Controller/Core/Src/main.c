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
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_ID 0b0001

#define CAN_ID_MASK 		0b00000000000011110000000000000000
#define CAN_ACTUATOR_MASK 	0b00000000000000001111000000000000

// The number of data bytes in the CAN data frames (float32 values).
#define CAN_DATA_SIZE 4

//Joystick dead zone size as ratio of joystick range (must be < 1 and positive).
#define DEAD_ZONE_SIZE 0.15
#define MOTOR_SPEED_MAX (4*M_PI)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc4;

CAN_HandleTypeDef hcan;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC4_Init(void);
/* USER CODE BEGIN PFP */


	float map(float x,float minVal, float maxVal, float nMin, float nMax);



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void CAN_Transmit(
    CAN_HandleTypeDef* hcan,
    CAN_TxHeaderTypeDef* CAN_TxHeader,
    uint32_t* CAN_TxMailbox);
// Initializes the CANBus filter for the board.
void CAN_Filter(CAN_HandleTypeDef* hcan, CAN_TxHeaderTypeDef* CAN_TxHeader);

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
  MX_CAN_Init();
  MX_ADC2_Init();
  MX_ADC4_Init();
  /* USER CODE BEGIN 2 */

  	uint8_t displaying_number = 0;
  	uint8_t displaying_number2 = 0;
  	int digits[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
  	uint8_t mode = 0;   // Variable that will store the current mode

  	CAN_TxHeaderTypeDef CAN_TxHeader; // The transmission header.
  	CAN_RxHeaderTypeDef CAN_RxHeader; // The receiver header.
  	uint32_t CAN_TxMailbox = 0;
  	uint8_t CAN_TxData[CAN_DATA_SIZE] = {};
  	uint8_t CAN_RxData[CAN_DATA_SIZE] = {};
  	CAN_Filter(&hcan, &CAN_TxHeader); // Initializing the CANbus filter
  	HAL_CAN_Start(&hcan);

  	uint8_t speedySelect = 0;
  	uint8_t actuatorSelect = 0;
  	uint8_t debugId = 15;
  	uint8_t priority = 0;
  	uint8_t commandId = 0x07;




  // Turn off all of the segments initially, then display 0 --> 0x7F
  HAL_GPIO_WritePin(GPIOA,0x7F,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA,digits[displaying_number], GPIO_PIN_RESET);

  // NEW code for 2nd display
  HAL_GPIO_WritePin(GPIOB,0x7F,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,digits[displaying_number2], GPIO_PIN_RESET);

  uint32_t readXval; // Joystick X-axis Value
  uint32_t readYval;
  HAL_ADC_Start(&hadc2); // Start ADC2 for the channel corresponding to PA7
  HAL_ADC_Start(&hadc4);
 // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

//read each pins of the slide switch to determine position of the switch, each position will correspond to a mode 1-4

	  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET){
		  mode = 1;
	  } else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)== GPIO_PIN_SET){
		  mode = 2;
	  } else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)== GPIO_PIN_SET){
		  mode = 3;
	 // } else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15)== GPIO_PIN_SET){
		//  mode = 4;
	  }

	  switch(mode)
	  {

	  case 1: {

// Incrementing button #1
	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET){
		  HAL_Delay(50);   // Debounce delay --> checking twice to ensure button is pressed

		  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET){
			  if(displaying_number < 9){
				  displaying_number++;
		  }

//Turn off all segments
			  HAL_GPIO_WritePin(GPIOA,0x7F,GPIO_PIN_SET);
			  HAL_Delay(10);
//Set new digit
			  HAL_GPIO_WritePin(GPIOA,digits[displaying_number],GPIO_PIN_RESET);
			  HAL_Delay(100);
		  	  }
	  }

	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) == GPIO_PIN_SET){

		  HAL_Delay(50);
		  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) == GPIO_PIN_SET){

			  if(displaying_number > 0){
				  displaying_number--;
		}

			  HAL_GPIO_WritePin(GPIOA,0x7F,GPIO_PIN_SET);
//Turn off all segments
			  HAL_Delay(10);
//Set new digit
			  HAL_GPIO_WritePin(GPIOA,digits[displaying_number],GPIO_PIN_RESET);
			  HAL_Delay(100);
}
}
// 2nd Button incrementer

	  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET){
	  		  HAL_Delay(50);   // Debounce delay --> checking twice to ensure button is pressed

	  		  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET){
	  			  if(displaying_number2 < 9){
	  				  displaying_number2++;
	  		  }

//Turn off all segments
	  			  HAL_GPIO_WritePin(GPIOB,0x7F,GPIO_PIN_SET);
	  			  HAL_Delay(10);
//Set new digit
	  			  HAL_GPIO_WritePin(GPIOB,digits[displaying_number2],GPIO_PIN_RESET);
	  			  HAL_Delay(100);
	  		  	  }
	  	  }

//2nd button decrementer
	  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_SET){

	  		  HAL_Delay(50);
	  		  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_SET){

	  			  if(displaying_number2 > 0){
	  				  displaying_number2--;
	  		}

	  			  HAL_GPIO_WritePin(GPIOB,0x7F,GPIO_PIN_SET);
// Turn off all segments
	  			  HAL_Delay(10);
//Set new digit
	  			  HAL_GPIO_WritePin(GPIOB,digits[displaying_number2],GPIO_PIN_RESET);
	  			  HAL_Delay(100);


	  }
	  }




	  			  HAL_ADC_PollForConversion(&hadc2, 100); // Wait for ADC2 to complete conversion
	  			  readXval = HAL_ADC_GetValue(&hadc2);  // Read the ADC value from PA7

	  			  HAL_ADC_PollForConversion(&hadc4, 100); // Wait for ADC4 to complete conversion
	  			  readYval = HAL_ADC_GetValue(&hadc4);  // Read the ADC value from PB15

	  			  float data = -map((float)readXval, 0, 200, -1, 1);

	  			  float newData = map((float)readYval, 0, 4095, -200, 200);   // for left joystick

	  			  if(data < -DEAD_ZONE_SIZE){
	  				  data = (data+DEAD_ZONE_SIZE)/(1-DEAD_ZONE_SIZE);
	  			  } else if(data > DEAD_ZONE_SIZE){
	  				  data = (data-DEAD_ZONE_SIZE)/(1-DEAD_ZONE_SIZE);;
	  			  } else {
	  				  data = 0;
	  			  }
	  			  //data *= MOTOR_SPEED_MAX;

	  			  if(-50 < newData && newData < 50){   // deadzone for left joystick
	  				  newData = 0;
	  			  }



	  			  uint32_t data2;
				  uint32_t newData2;

	  			  memcpy(&newData2, &newData, sizeof newData2);  // left joystick
	  			  memcpy(&data2, &data, sizeof data2);

	  			 speedySelect = displaying_number2;
	  			 actuatorSelect = displaying_number;

	  			 CAN_TxData[0] = data2>>24;
	  			 CAN_TxData[1] = data2>>16;
	  			 CAN_TxData[2] = data2>>8;
	  			 CAN_TxData[3] = data2;


	  			 //CAN_TxData[4] = newData2>>24;
	  			 //CAN_TxData[5] = newData2>>16;
	  			 //CAN_TxData[6] = newData2>>8;
	  		     //CAN_TxData[7] = newData2;



	  			 CAN_TxHeader.ExtId = priority<<24|commandId<<16|speedySelect<<12|actuatorSelect<<8|debugId<<4;

	  			 HAL_CAN_AddTxMessage(&hcan, &CAN_TxHeader,CAN_TxData, &CAN_TxMailbox);






	  	  }

	  	  break;





	  case 2:
		  break;
	  case 3:
		  break;
	  case 4:
		  break;
	  default:
		  break;


  }

  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12|RCC_PERIPHCLK_ADC34;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.Adc34ClockSelection = RCC_ADC34PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_8B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC4_Init(void)
{

  /* USER CODE BEGIN ADC4_Init 0 */

  /* USER CODE END ADC4_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC4_Init 1 */

  /* USER CODE END ADC4_Init 1 */

  /** Common config
  */
  hadc4.Instance = ADC4;
  hadc4.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc4.Init.Resolution = ADC_RESOLUTION_12B;
  hadc4.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc4.Init.ContinuousConvMode = DISABLE;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc4.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.NbrOfConversion = 1;
  hadc4.Init.DMAContinuousRequests = DISABLE;
  hadc4.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc4.Init.LowPowerAutoWait = DISABLE;
  hadc4.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC4_Init 2 */

  /* USER CODE END ADC4_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB3
                           PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB7
                           PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_7
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

float map(float x,float minVal, float maxVal, float nMin, float nMax){


	 if (maxVal == minVal) return 0;

	    // Apply the mapping formula
	    return nMin + ((x - minVal) * (nMax - nMin)) / (maxVal - minVal);



}



void CAN_Filter(CAN_HandleTypeDef* hcan, CAN_TxHeaderTypeDef* CAN_TxHeader)
{ // This function initializes the CAN filter for the board.
  // The CANID ports are named for their respective address bits, i.e., 0 to the 0th bit.

    // Setting up the TxHeader
    CAN_TxHeader->IDE = CAN_ID_EXT; // Extended identifier, not the standard length.
    CAN_TxHeader->RTR = CAN_RTR_DATA; // Specifying data frames, not remote frames.
    CAN_TxHeader->DLC = CAN_DATA_SIZE; // CAN_SIZE_DATA; //The data size (5 bytes)
    CAN_TxHeader->ExtId = 0; // Needs to be changed depending on the frame

    // Setting the extended transmission header based on the CAN pins

    CAN_TxHeader->ExtId |= (uint32_t)(CAN_ID << 12);

    CAN_FilterTypeDef CAN_FILTER_CONFIG; // Declaring the filter structure.
    CAN_FILTER_CONFIG.FilterFIFOAssignment = CAN_FILTER_FIFO0; // Choosing the FIFO0 set.
    CAN_FILTER_CONFIG.FilterIdHigh = (uint32_t)(CAN_ID >> 1);
    CAN_FILTER_CONFIG.FilterIdLow = (uint32_t)((CAN_ID << 15) & 0xFFFF);
    CAN_FILTER_CONFIG.FilterMaskIdHigh = (uint32_t)(CAN_ID_MASK >> 16);
    CAN_FILTER_CONFIG.FilterMaskIdLow = (uint32_t)(CAN_ID_MASK & 0xFFFF);
    CAN_FILTER_CONFIG.FilterBank = 0;
    CAN_FILTER_CONFIG.FilterMode = CAN_FILTERMODE_IDMASK; // Using the mask mode to ignore certain bits.
    CAN_FILTER_CONFIG.FilterScale = CAN_FILTERSCALE_32BIT; // Using the extended ID so 32bit filters.
    CAN_FILTER_CONFIG.FilterActivation = CAN_FILTER_ENABLE; // Enabling the filter.
    HAL_CAN_ConfigFilter(hcan, &CAN_FILTER_CONFIG);
}
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
