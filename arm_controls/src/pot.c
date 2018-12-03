#include "stm32f1xx.h"
#include "stm32f1xx_nucleo.h"
#include "stm32f1xx_hal.h"

#include "pot.h"
static void __MX_ADC_Init();

void potInit(){
	__MX_ADC_Init();
}

/**
 * returns a number between [0, x) representing the turn
 * returns -1 in an error
 */
int readPot(){
	static int val;
	val = -1;

	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK){
		val = HAL_ADC_GetValue(&hadc1);
	}
    HAL_ADC_Stop(&hadc1);

	return val;
}

static void __MX_ADC_Init(){
	ADC_ChannelConfTypeDef sConfig;
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;

	HAL_ADC_Init(&hadc1);

	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
