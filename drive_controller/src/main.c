/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f3xx.h"

#include "main.h"
#include "coms.h"
#include "sabertooth.h"


UART_HandleTypeDef huart;

void serial_write8(uint8_t data) {
	// Write a byte out onto UART
	HAL_UART_Transmit(&huart, &data, 1, 1000);
}

void uart_init() {
	huart.Instance 			= USART3;
	huart.Init.BaudRate		= 9600; // Should this be a different value?
	huart.Init.WordLength 	= UART_WORDLENGTH_8B;
	huart.Init.StopBits 	= UART_STOPBITS_1;
	huart.Init.Parity		= UART_PARITY_NONE;
	huart.Init.Mode			= UART_MODE_TX;
	huart.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	HAL_UART_Init(&huart);
}

// Using USART3, on PB10
void HAL_UART_MspInit(UART_HandleTypeDef* instance) {
	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_10;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOB, &gpio);
}

void motors_init() {
	saberA.write8 = serial_write8;
	saberA.address = 128;
	saberA.deadband = 0;
	saberA.min_voltage = 16;
	saberA.max_voltage = 30;
	saberA.ramp_setting = 30;

	saberB.write8 = serial_write8;
	saberB.address = 129;
	saberB.deadband = 0;
	saberB.min_voltage = 16;
	saberB.max_voltage = 30;
	saberB.ramp_setting = 30;

	sabertooth_init(&saberA);
	sabertooth_init(&saberA);
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16; // ???
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	clocks_init();
	uart_init();
	libcanard_init();
	motors_init();
	// Start the timeout thing
	timeout = HAL_GetTick();


	for(;;) {
		static uint32_t last_thing = 0;
		handle_frame(); // literally nothing else to do
		tx_once();

		if ((HAL_GetTick() - last_thing) > 1000) {
			last_thing = HAL_GetTick();

			coms_send_NodeStatus(UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
				UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL, 0);

		}

		if ((HAL_GetTick() - timeout) > MOTOR_TIMEOUT_MS) {
			sabertooth_set_motor(&saberA, 0, 0);
			sabertooth_set_motor(&saberA, 1, 0);
		}

	}
}
