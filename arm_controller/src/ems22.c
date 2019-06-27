/*
 * ems22.c
 *
 *  Created on: Feb 23, 2019
 *      Author: isthatme
 */

#include <stm32f3xx.h>

static SPI_HandleTypeDef spi;

void ems22_init() {
	spi.Instance = SPI1;
	spi.Init.Mode = SPI_MODE_MASTER;
	spi.Init.Direction = SPI_DIRECTION_2LINES;
	spi.Init.DataSize = SPI_DATASIZE_16BIT;
	spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
	spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	spi.Init.NSS = SPI_NSS_SOFT;

	HAL_SPI_Init(&spi);
}

