#include "main.h"

/*** @brief General GPIO Initialization
 * 
 */
void gpio_init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* ------ Inputs ------ */
	// Node select
	GPIO_InitTypeDef gpio;
	gpio.Pin = NODE_SELECT_PIN0 | NODE_SELECT_PIN1 | NODE_SELECT_PIN2 | NODE_SELECT_PIN3;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &gpio);

	// User Button
	gpio.Pin = USR_BTN_PIN;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USR_BTN_PORT, &gpio);

	// Analog Inputs
	gpio.Pin = CURRENT_MSR_PIN | VOLTAGE_MSR_PIN;
	gpio.Pull = GPIO_NOPULL;
	gpio.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(CURRENT_MSR_PORT, &gpio);

	/* ------- Outputs ------ */
	// Out Enable
	gpio.Pin = OUT_EN_PIN;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OUT_EN_PORT, &gpio);

	// CAN Bus
	gpio.Pin = CAN_RX_PIN | CAN_TX_PIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF9_CAN;
	HAL_GPIO_Init(CAN_PORT, &gpio);
}

/*
 * Theoretically a working blinky example.
 * Untested though.
 */
int main(void) {
	clocks_init();
	gpio_init();
	coms_init();
	for (;;) {
	}
}


