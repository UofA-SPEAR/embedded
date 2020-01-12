/*
 * ems22.c
 *
 *  Created on: Feb 23, 2019
 *      Author: isthatme
 */

#include <stm32f3xx.h>
#include <string.h>

static SPI_HandleTypeDef spi;

void ems22_init() {
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// Initialise pins for SPI comms
	GPIO_InitTypeDef gpio;
	gpio.Pin 		= GPIO_PIN_3 | GPIO_PIN_4;
	gpio.Mode 		= GPIO_MODE_AF_PP;
	gpio.Alternate 	= GPIO_AF5_SPI1;
	gpio.Pull 		= GPIO_NOPULL;
	gpio.Speed		= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOB, &gpio);

	// Initialise chip select pins
	gpio.Pin		= GPIO_PIN_11 | GPIO_PIN_12;
	gpio.Mode 		= GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOC, &gpio);

	// Disable CS by default, active low
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, 1);

	spi.Instance = SPI1;
	spi.Init.Mode = SPI_MODE_MASTER;
	spi.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
	spi.Init.DataSize = SPI_DATASIZE_16BIT;
	spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
	spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	spi.Init.NSS = SPI_NSS_SOFT;
	spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi.Init.TIMode = SPI_TIMODE_DISABLE; // Suspect
	spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

	HAL_SPI_Init(&spi);
}


int16_t ems22_read_position(uint8_t motor) {
	struct {
		uint16_t abs_position : 10;
		uint8_t offset_compensation : 1;
		uint8_t cordic_overflow : 1;
		uint8_t linearity_alarm : 1;
		uint8_t magnitude_increase : 1;
		uint8_t magnitude_decrease : 1;
		uint8_t even_parity : 1;
	} in_data;

	// Pull CS pin low
	HAL_GPIO_WritePin(GPIOC, (GPIO_PIN_11 << motor), 0);
	
	HAL_SPI_Receive(&spi, (uint8_t*) &in_data, 2, 100);

	HAL_GPIO_WritePin(GPIOC, (GPIO_PIN_11 << motor), 1);

	{
		uint16_t parity_check;
		uint8_t bit_count = 0;

		memcpy((void*) &parity_check, (void*) &in_data, 2);

		// Count bits
		while (parity_check) {
			bit_count += parity_check & 1;
			parity_check = parity_check >> 1;
		}

		// Parity error
		if ((parity_check % 2) == in_data.even_parity) {
			return -1;
		}
	}

	// If no errors, return data
	return in_data.abs_position;
}