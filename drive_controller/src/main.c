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
#include "stm32f3xx_nucleo_32.h"

#include "coms.h"
#include "sabertooth.h"

sabertooth_t saberA, saberB;

UART_HandleTypeDef huart;

void serial_write8(uint8_t data) {
	// Write a byte out onto UART
	HAL_UART_Transmit(&huart, &data, 1, 1000);
}

void uart_init() {
	huart.Instance 			= USART1;
	huart.Init.BaudRate		= 9600; // Should this be a different value?
	huart.Init.WordLength 	= UART_WORDLENGTH_8B;
	huart.Init.StopBits 	= UART_STOPBITS_1;
	huart.Init.Parity		= UART_PARITY_NONE;
	huart.Init.Mode			= UART_MODE_TX;
	huart.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	HAL_UART_Init(&huart);
}

// On the nucleo, UART1 TX is PA9 and RX is PA10
void HAL_UART_MspInit(UART_HandleTypeDef* instance) {
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_9;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &gpio);
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

int main(void)
{
	uart_init();
	libcanard_init();
	motors_init();


	for(;;) {
		rx_once(); // literally nothing else to do
	}
}
