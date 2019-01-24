#include "stm32f1xx.h"
#include "stm32f1xx_nucleo.h"
#include "stm32f1xx_hal.h"

#include "motor.h"
static void __MX_TIM2_Init(void);

void motorSet(int speed, enum Direction dir) {
	if (dir == COAST) {
		// TODO make this not hardcoded
		TIM2->CCR2 = 1000;
	} else {
		TIM2->CCR2 = speed;
	}

	// Pull both bridges high. We don't want to short the input power
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INB_PIN, GPIO_PIN_SET);

	/* Assuming that output A is the "positive" terminal on the motor.
	 *
	 * We pull B side low so current flows from A to B for forwards,
	 * and we pull A side low for current from B to A going backwards.
	 *
	 * When we coast we leave both sides high, which allows current to flow through
	 * the power path. We prefer this to the low side because we don't inject noise
	 * into ground.
	 *
	 * Braking is the same but we selectively turn off the power so the chip absorbs some power.
	 */
	switch (dir) {
	case (FORWARD):
			HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INB_PIN, GPIO_PIN_RESET);
			break;
	case (REVERSE):
			HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INA_PIN, GPIO_PIN_RESET);
			break;
	// Do nothing, already covered these cases with the safety thing.
	default:
			break;
	}
}

void motorEnable(int enable) {
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_ENA_PIN, enable);
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_ENB_PIN, enable);
}

// setup code
void motorInit() {
	__MX_TIM2_Init();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Start with all pins low
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INA_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_INB_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_ENA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_ENB_PIN, GPIO_PIN_SET);

	// Initialize pins
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = MOTOR_INA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = MOTOR_INA_PIN;
	HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = MOTOR_ENA_PIN;
	HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = MOTOR_ENB_PIN;
	HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);
}

/* TIM2 init function
 * Sets up timers
 */
static void __MX_TIM2_Init(void) {
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_PWM_Init(&htim2);

	TIM_OC_InitTypeDef itd;
	itd.OCMode = TIM_OCMODE_PWM1;
	itd.OCFastMode = TIM_OCFAST_DISABLE;
	itd.OCIdleState = TIM_OCIDLESTATE_SET;
	itd.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	itd.OCPolarity = TIM_OCPOLARITY_HIGH;
	itd.OCNPolarity = TIM_OCNPOLARITY_LOW;
	itd.Pulse = 0;

	HAL_TIM_OC_ConfigChannel(&htim2, &itd, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

}


// Initialize GPIO for PWM output
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = MOTOR_PWM_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);
}
