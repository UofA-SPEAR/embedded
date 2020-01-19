/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "../Inc/main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "minmea-master/minmea.h"
#include "math.h"
#include "../Inc/libcanard/canard.h"
#include "../Inc/libcanard/drivers/stm32/canard_stm32.h"
#include "libcanard_wrapper.h"
#include "../Inc/libcanard_dsdlc_generated/uavcan/equipment/gnss/Fix2.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define LINEMAX 200 // Maximal allowed/expected line length
#define USART_GPS USART1
#define USART_GPS_IRQn USART1_IRQn
#define USART_GPS_IRQHandler USART1_IRQHandler
#define ONEe8 100000000
#define INDENT_SPACES "  "
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLOCK_RATE 8000000
#define CAN_RATE 250000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static char gpsBuffer[LINEMAX + 1];
uavcan_equipment_gnss_Fix2 gpsData;
uint8_t gpsMsgBuffer[UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE];
uint32_t inout_transfer_id;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void USART_GPS_IRQHandler(void);
void gps_parse_buffer(void);
void gps_send_frame(void);
void on_reception(CanardInstance* ins,                		///< Library instance
                        CanardRxTransfer* transfer);
bool should_accept(const CanardInstance* ins,         		///< Library instance
                        uint64_t* out_data_type_signature,  ///< Must be set by the application!
                        uint16_t data_type_id,              ///< Refer to the specification
						CanardTransferType transfer_type,   ///< Refer to CanardTransferType
                        uint8_t source_node_id);            ///< Source node ID or Broadcast (0)
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Written by Clive Two.Zero found from https://community.st.com/s/feed/0D50X00009XkVxKSAV
void USART_GPS_IRQHandler(void) // Sync and Queue NMEA Sentences
{
	__disable_irq();
	static int rx_index = 0;
	if (USART_GPS->ISR & USART_ISR_ORE) // Overrun Error
		USART_GPS->ICR = USART_ICR_ORECF;
	if (USART_GPS->ISR & USART_ISR_NE) // Noise Error
		USART_GPS->ICR = USART_ICR_NCF;
	if (USART_GPS->ISR & USART_ISR_FE) // Framing Error
		USART_GPS->ICR = USART_ICR_FECF;
	if (USART_GPS->ISR & USART_ISR_RXNE) // Received character?
	{
		char rx = (char)(USART_GPS->RDR & 0xFF);
		if ((rx == '\r') || (rx == '\n')) // Is this an end-of-line condition, either will suffice?
		{
			if (rx_index != 0) // Line has some content?
			{
				gpsBuffer[rx_index++] = 0; // Add NUL if required down stream
				QueueBuffer(gpsBuffer, rx_index); // Copy to queue from live dynamic receive buffer
				rx_index = 0; // Reset content pointer
			}
		}
		else
		{
			if ((rx == '$') || (rx_index == LINEMAX)) // If resync or overflows pull back to start
				rx_index = 0;
			gpsBuffer[rx_index++] = rx; // Copy to buffer and increment
		}
	}
	__enable_irq();
}

void gps_parse_buffer(void)
{
	switch (minmea_sentence_id(gpsBuffer, false))
    {
		case MINMEA_SENTENCE_RMC:
		{
			struct minmea_sentence_rmc frame;
		    if (minmea_parse_rmc(&frame, gpsBuffer))
		    {
		    	gpsData.longitude_deg_1e8 = frame.longitude.value / frame.longitude.scale * ONEe8;
		    	gpsData.latitude_deg_1e8 = frame.latitude.value / frame.latitude.scale * ONEe8;
		    	gpsData.ned_velocity[0] = frame.speed.value / frame.speed.scale * cos(frame.course);
		    	gpsData.ned_velocity[1] = frame.speed.value / frame.speed.scale * sin(frame.course);
		    	gpsData.ned_velocity[2] = 0;
		    	gpsData.gnss_time_standard = frame.time.hours * 10000 + frame.time.minutes * 100 + frame.time.seconds;
		    }
		    else
		    {
		        printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
		    }
		} break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, gpsBuffer))
            {
            // TODO: fix quality msg?
            }
            else
            {
            	printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
	        }
	    } break;

        case MINMEA_INVALID:
        {
            printf(INDENT_SPACES "$xxxxx sentence is not valid\n");
        } break;

        default:
        {
            printf(INDENT_SPACES "$xxxxx sentence is not parsed\n");
        } break;
	}
}

void gps_send_frame(void) {
	// init msg datatype
	// encode gpsData with function
	uint8_t len = uavcan_equipment_gnss_Fix2_encode(&gpsData, gpsMsgBuffer);
	// broadcast
	canardBroadcast(&m_canard_instance,
			UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE,
			UAVCAN_EQUIPMENT_GNSS_FIX2_ID,
				&inout_transfer_id,
				0,
				gpsMsgBuffer,
				len);
}

void on_reception(CanardInstance* ins,                		///< Library instance
                        CanardRxTransfer* transfer)
{
	// do nothing
	return;
}

bool should_accept(const CanardInstance* ins,         		///< Library instance
                        uint64_t* out_data_type_signature,  ///< Must be set by the application!
                        uint16_t data_type_id,              ///< Refer to the specification
						CanardTransferType transfer_type,   ///< Refer to CanardTransferType
                        uint8_t source_node_id)             ///< Source node ID or Broadcast (0)
{
	return false;
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  libcanard_init(on_reception, should_accept, CLOCK_RATE, CAN_RATE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (strcmp(gpsBuffer[0], '0') != 0)
	  {
		  gps_parse_buffer();
		  gps_send_frame();
	  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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
  huart1.Init.BaudRate = 4800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  NVIC_EnableIQR(USART1_IRQn);
  HAL_NVIC_SetPriority(GPS_USART_IRQn, 1, 0);
  USART_GPS->CR1 |= USART_CR1_RXNEIE; // Enable Interrupt
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
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  VIC_EnableIQR(USART2_IRQn);
  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
