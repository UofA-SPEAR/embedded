#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"


/*
 * Theoretically a working blinky example.
 * Untested though.
 */
int main(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOA, &gpio);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_Init();
    
	for (;;) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
		HAL_Delay(500);
	}
}


