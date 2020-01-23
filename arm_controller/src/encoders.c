#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "encoders.h"

ADC_HandleTypeDef hadc2;
// I hope TIM3 isn't enabled anywhere else
TIM_HandleTypeDef tim3;

void pot_init(uint8_t motor) {
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc2.Init.Resolution = ADC_RESOLUTION_12B;
	hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc2.Init.LowPowerAutoWait = DISABLE;
	hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;


	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		while(1);
	}

	// Configure ADC multi-mode
	ADC_MultiModeTypeDef multimode;
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc2, &multimode) != HAL_OK) {
		while(1);
	}

	/* Motor 0 is PA6, motor 1 is PA7
	 * channels 3 and 4, respectively
	 */
	if (motor == 0) {
		// Configure channel sample time to be 1.5 cycles
		ADC2->SMPR1 |= (0b000) << ADC_SMPR1_SMP3_Pos;
		// Channel is single ended by default
	} else if (motor == 1) {
		// Configure channel sample time to be 1.5 cycles
		ADC2->SMPR1 |= (0b000) << ADC_SMPR1_SMP4_Pos;
		// Channel is single ended by default
	}

	// Configure GPIO
	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_6 << motor;
	gpio.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &gpio);
}

/**
 * returns a number between [0, x) representing the turn
 * returns -1 in an error
 */
uint32_t pot_read(uint8_t motor) {
	static int val;
	val = -1;

	// Configure channel sequence for motor
	ADC2->SQR1 = (3 + motor) << ADC_SQR1_SQ1_Pos;

	HAL_ADC_Start(&hadc2);
	if (HAL_ADC_PollForConversion(&hadc2, 1000) == HAL_OK){
		val = HAL_ADC_GetValue(&hadc2);
	}
    HAL_ADC_Stop(&hadc2);

	return val;
}

static void encoder_gpio_init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_TIM8_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_8;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_PULLUP;
	gpio.Alternate = GPIO_AF10_TIM8;

	HAL_GPIO_Init(GPIOB, &gpio);
}

/** @brief Set up TIM3 in encoder mode.
 * 
 * Note here that the pins are not correct for encoder A to work
 */
void encoder_init() {
	encoder_gpio_init();

	tim3.Instance = TIM8;
	tim3.Init.Prescaler = 0;
	tim3.Init.Period = UINT16_MAX;
	tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	TIM_Encoder_InitTypeDef encoder;
	encoder.EncoderMode = TIM_ENCODERMODE_TI12;
	// Theoretically could get more resolution doing botheddge,
	// but depends on the encoder
	encoder.IC1Polarity 	= TIM_ICPOLARITY_RISING;
	encoder.IC1Selection 	= TIM_ICSELECTION_DIRECTTI;
	encoder.IC1Filter 		= 0; // Assume it's pretty good
	encoder.IC1Prescaler	= TIM_ICPSC_DIV1;
	encoder.IC2Polarity 	= TIM_ICPOLARITY_RISING;
	encoder.IC2Selection 	= TIM_ICSELECTION_DIRECTTI;
	encoder.IC2Filter 		= 0; // Assume it's pretty good
	encoder.IC2Prescaler	= TIM_ICPSC_DIV1;

	// Start counter at the middle so we can go negative

	HAL_TIM_Encoder_Init(&tim3, &encoder);
	HAL_TIM_Encoder_Start(&tim3, TIM_CHANNEL_ALL);
	TIM8->CNT = ENCODER_START_VAL;

	TIM_MasterConfigTypeDef sMasterConfig;


	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&tim3, &sMasterConfig);
}

/*
 * Porting ADC Capabilities to ChibiOS
 */

#include "ch.h"
#include "hal.h"

adcInit();
adcStart();

static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_GRP1_NUM_CHANNELS,
  NULL,
  adcerrorcallback,
  ADC_CFGR_CONT,            /* CFGR    */
  ADC_TR(0, 4095),          /* TR1     */
  ADC_CR2_SWSTART,          /* Cr2: SOFTWARE TRIGGER*/
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_61P5),
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2),
    0,
    0,
    0
  }
};
/**
 * @brief   Performs an ADC conversion.
 * @details Performs a synchronous conversion operation.
 * @note    The buffer is organized as a matrix of M*N elements where M is the
 *          channels number configured into the conversion group and N is the
 *          buffer depth. The samples are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] grpp      pointer to a @p ADCConversionGroup object
 * @param[out] samples  pointer to the samples buffer
 * @param[in] depth     buffer depth (matrix rows number). The buffer depth
 *                      must be one or an even number.
 * @return              The operation result.
 * @retval MSG_OK       Conversion finished.
 * @retval MSG_RESET    The conversion has been stopped using
 *                      @p acdStopConversion() or @p acdStopConversionI(),
 *                      the result buffer may contain incorrect data.
 * @retval MSG_TIMEOUT  The conversion has been stopped because an hardware
 *                      error.
 *
 * @api
 */
msg_t adcConvert(ADCDriver *adcp,
                 const ADCConversionGroup *grpp,
                 adcsample_t *samples,
                 size_t depth) {
  ...
}

/**
 * @brief   Stops an ongoing conversion.
 * @details This function stops the currently ongoing conversion and returns
 *          the driver in the @p ADC_READY state. If there was no conversion
 *          being processed then the function does nothing.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @api
 */
void adcStopConversion(ADCDriver *adcp) {
  ...
}

