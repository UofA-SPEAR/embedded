#include "stm32f1xx.h"
#include "stm32f1xx_nucleo.h"
#include "stm32f1xx_hal.h"

#include "motor.h"
static void __MX_TIM2_Init(void);


/**
 * Speed is between 0 for stop, and 1000 for max
 */
void setMotor(int speed, enum Direction dir){
	HAL_GPIO_WritePin(GPIOA, DIR_PIN, dir);
	TIM2->CCR2 = speed;
}

void motorInit(){
	__MX_TIM2_Init();
	__HAL_RCC_GPIOA_CLK_ENABLE();


	// init pins
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = DIR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* TIM2 init function */
static void __MX_TIM2_Init(void)
{

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

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin =  PWM_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
