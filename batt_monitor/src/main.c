#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"


/*** @brief General GPIO Initialization
 * 
 */
void gpio_init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* ------ Inputs ------ */
	// Node select
	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &gpio);

	// User Button
	gpio.Pin = GPIO_PIN_1;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &gpio);

	// Analog Inputs

	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_2;
	gpio.Pull = GPIO_NOPULL;
	gpio.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &gpio);

	/* ------- Outputs ------ */
	// Out Enable
	gpio.Pin = GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &gpio);

	// CAN Bus
	gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF7_CAN; // TODO verify this
	HAL_GPIO_Init(GPIOA, &gpio);
}

/*
 * Theoretically a working blinky example.
 * Untested though.
 */
int main(void) {
	for (;;) {
	}
}


