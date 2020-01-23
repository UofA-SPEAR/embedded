#include "drv8701.h"
#include "hal.h"

#define DRV8701_PH_PORT GPIOB
#define DRV8701_PH_PIN 8
#define DRV8701_PH_MODE PAL_MODE_OUTPUT_PUSHPULL

#define DRV8701_EN_PORT GPIOB
#define DRV8701_EN_PIN 9
#define DRV8701_EN_MODE PAL_MODE_ALTERNATE(1)

#define DRV8701_nFAULT_PORT GPIOB
#define DRV8701_nFAULT_PIN 7
#define DRV8701_nFAULT_MODE PAL_MODE_INPUT_PULLUP

#define DRV8701_nSLEEP_PORT GPIOB
#define DRV8701_nSLEEP_PIN 5
#define DRV8701_nSLEEP_MODE PAL_MODE_PUSHPULL

#define DRV8701_SNSOUT_PORT GPIOB
#define DRV8701_SNSOUT_PIN 14
#define DRV8701_SNSOUT_MODE PAL_MODE_INPUT_PULLUP


#define DRV8701_VREF_PORT GPIOA
#define DRV8701_VREF_PIN 4
#define DRV8701_VREF_MODE PAL_MODE_ANALOG

#define DRV8701_SO_PORT GPIOA
#define DRV8701_SO_PIN 1
#define DRV8701_SO_MODE PAL_MODE_ANALOG

static const PWMConfig pwmcfg = {
	4000000, // 1MHz Timer Frequency
	500, // period is 500us to set the PWM signal to 80kHz
	NULL,
	{
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_ACTIVE_HIGH,NULL},
	},
	0,
	0
};

static const DACConfig dac1cfg1 = {
	.init = 4047U,
	.datamode = DAC_DHRM_12BIT_RIGHT
};

void drv8701_init(void)
{
	palSetMode(DRV8701_PH_PORT, DRV8701_PH_PIN, DRV8701_PH_MODE);
	palSetMode(DRV8701_EN_PORT, DRV8701_EN_PIN, DRV8701_EN_MODE);
	palSetMode(DRV8701_nFAULT_PORT, DRV8701_nFAULT_PIN, DRV8701_nFAULT_MODE);
	palSetMode(DRV8701_nSLEEP_PORT, DRV8701_nSLEEP_PIN, DRV8701_nSLEEP_MODE);
	palSetMode(DRV8701_SNSOUT_PORT, DRV8701_SNSOUT_PIN, DRV8701_SNSOUT_MODE);
	palSetMode(DRV8701_VREF_PORT, DRV8701_VREF_PIN, DRV8701_VREF_MODE);
	palSetMode(DRV8701_SO_PORT, DRV8701_SO_PIN, DRV8701_SO_MODE);


	// Start PWM and DAC
	pwmStart(&PWMD4, &pwmcfg);
	dacStart(&DACD1, &dac1cfg1);

	// enable TIM4 Channel 4
	pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 0));

	// Initialize the ADC
	/*
	* Activates the ADC1 driver and the temperature sensor.
	*/
	adcStart(&ADCD1, NULL);
	adcSTM32EnableTS(&ADCD1);
	adcSTM32EnableVBAT(&ADCD1);

	/*
	* Linear conversion.
	*/
	adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
	chThdSleepMilliseconds(1000);
}

void drv8701_set(int16_t velocity)
{
	unit16_t speed = abs(velocity);

	if(speed > 10000)
		speed = 10000;

	if(velocity > 0) {
		palSetPad(DRV8701_PH_PORT,DRV8701_PH_PIN);
		palSetPad(DRV8701_nSLEEP_PORT,DRV8701_nSLEEP_PIN);
		pwmChangePeriodI(&PWMD4, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, speed));
	} else {
		palClearPad(DRV8701_PH_PORT,DRV8701_PH_PIN);
		palSetPad(DRV8701_nSLEEP_PORT,DRV8701_nSLEEP_PIN);
		pwmChangePeriodI(&PWMD4, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, speed));
	}
}

void drv8701e_stop(void)
{
	palClearPad(DRV8701_nSLEEP_PORT, DRV8701_nSLEEP_PIN);
}

void drv8701_set_current(dacsample_t current)
{
	if(current < 5) {
		//ToDo: send error message through canbus
	}

	//write to DAC
	current = current * 10 + 5;
	current *= (4095u/330u);
	dacPutChannelX(&DACD1, 0, current);
}