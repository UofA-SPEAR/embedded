#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "vnh5019.h"

static void vnh5019_pwm_init(vnh5019_t* settings);

uint8_t timer_initialized = 0;

void motor_set(vnh5019_t* motor, int speed, enum Direction dir) {
	if (dir == COAST) {
		// TODO make this not hardcoded
		VNH5019_TIM_INSTANCE->CCR2 = 1000;
	} else {
		VNH5019_TIM_INSTANCE->CCR2 = speed;
	}

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
			HAL_GPIO_WritePin(motor->digital.port, motor->digital.in_a, GPIO_PIN_SET);
			HAL_GPIO_WritePin(motor->digital.port, motor->digital.in_b, GPIO_PIN_RESET);
			break;
	case (REVERSE):
			HAL_GPIO_WritePin(motor->digital.port, motor->digital.in_a, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(motor->digital.port, motor->digital.in_b, GPIO_PIN_SET);
			break;
	// Do nothing, already covered these cases with the safety thing.
	default:
			break;
	}
}

void motor_enable(vnh5019_t* motor, uint8_t enable) {
	if (enable) {
		HAL_GPIO_WritePin(motor->digital.port, motor->digital.en_a, GPIO_PIN_SET);
		HAL_GPIO_WritePin(motor->digital.port, motor->digital.en_b, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(motor->digital.port, motor->digital.en_a, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(motor->digital.port, motor->digital.en_b, GPIO_PIN_RESET);
	}
}


// setup code
void vnh5019_init(vnh5019_t* settings) {
	// Enable timer clock
	VNH5019_TIM_CLK_ENABLE();
	// Enable GPIO clocks (A, B, and C just to be sure)
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// Initialize PWM pins
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin 		= settings->pwm.pin;
	GPIO_InitStruct.Alternate 	= settings->pwm.tim_af;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull 		= GPIO_NOPULL;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(settings->pwm.port, &GPIO_InitStruct);

	// Initialize timer and channelf for PWM.
	vnh5019_pwm_init(settings);

	// Start with all pins low
	HAL_GPIO_WritePin(settings->digital.port, settings->digital.in_a, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(settings->digital.port, settings->digital.in_b, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(settings->digital.port, settings->digital.en_a, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(settings->digital.port, settings->digital.en_b, GPIO_PIN_RESET);

	// Initialize pins

	GPIO_InitStruct.Pin = MOTOR_INA_PIN | MOTOR_INB_PIN |
			MOTOR_ENA_PIN | MOTOR_ENB_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(settings->digital.port, &GPIO_InitStruct);
}


// Pins and clocks HAVE to be configured before this is called.
static void vnh5019_pwm_init(vnh5019_t* settings) {
	if (!timer_initialized) {
		// Initialize timer for PWM
		h_timer.Instance = VNH5019_TIM_INSTANCE;
		h_timer.Init.Prescaler = 1;
		h_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
		h_timer.Init.Period = 1000;
		h_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		h_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		HAL_TIM_PWM_Init(&h_timer);

		// Make timer run independently
		TIM_MasterConfigTypeDef sMasterConfig;
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		HAL_TIMEx_MasterConfigSynchronization(&h_timer, &sMasterConfig);

		timer_initialized = 1;
	}

	// Configure output compare channel to actually use PWM
	TIM_OC_InitTypeDef itd;
	itd.OCMode = TIM_OCMODE_PWM1;
	itd.OCFastMode = TIM_OCFAST_DISABLE;
	itd.OCIdleState = TIM_OCIDLESTATE_SET;
	itd.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	itd.OCPolarity = TIM_OCPOLARITY_HIGH;
	itd.OCNPolarity = TIM_OCNPOLARITY_LOW;
	itd.Pulse = 0;
	HAL_TIM_OC_ConfigChannel(&h_timer, &itd, settings->pwm.tim_ch);
	HAL_TIM_PWM_Start(&h_timer, settings->pwm.tim_ch);
}
