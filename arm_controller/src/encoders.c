#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "encoders.h"

ADC_HandleTypeDef hadc3;

void potInit(){
	__MX_ADC_Init();
}

void potA_init(void) {
	ADC_ChannelConfTypeDef sConfig;
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc3.Init.LowPowerAutoWait = DISABLE;
	hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

	if (HAL_ADC_Init(&hadc3) != HAL_OK) {
		while(1);
	}

	// Configure ADC multi-mode
	ADC_MultiModeTypeDef multimode;
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK) {
		while(1);
	}


	sConfig.Channel = ADC_CHANNEL_12;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;

	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
		while(1);
	}
}

/**
 * returns a number between [0, x) representing the turn
 * returns -1 in an error
 */
uint32_t potA_read(){
	static int val;
	val = -1;

	HAL_ADC_Start(&hadc3);
	if (HAL_ADC_PollForConversion(&hadc3, 1000) == HAL_OK){
		val = HAL_ADC_GetValue(&hadc3);
	}
    HAL_ADC_Stop(&hadc3);

	return val;
}

// more setup code, this time for the pin
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
	if (hadc->Instance == ADC3) {
		__HAL_RCC_ADC34_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = POTA_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(POTA_PORT, &GPIO_InitStruct);
	}
}
