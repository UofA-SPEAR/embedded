/** This is a really basic example of using digital pins on the 
SPEAR SPEEDY such as polling input **/


// From the Blinky demo, now we know how to blink an LED, let's go for LED Phasing at pin A4 using TIM3_CH2

#include "ch.h"
#include "hal.h"
PWMConfig pwmcfg = {
	100000,  // 100kHz Timer Frequency
	100,      // period is 500us to set the PWM frequency to 1kHz
	NULL,
	{
		{PWM_OUTPUT_DISABLED, NULL}, // This means disable CH1
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}, // CH2
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
	},
	0,
	0
};
int main(void)
{
	// These are mandatory for the project
	chSysInit();
	halInit();
	// Configure A4 as Alternate Mode 2 (AF2) refers to Pg.42 in the Datasheet
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_ALTERNATE(2));
	pwmStart(&PWMD3, &pwmcfg);
	uint16_t counter = 0;
	while(1) {
		pwmEnableChannel(&PWMD3, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, counter++));
		if(counter >= 10000) counter = 0;
		chThdSleep(1);
	}
}


