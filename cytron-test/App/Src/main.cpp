#include "ch.h"
#include "hal.h"
PWMConfig pwmcfg = {
        1000000,  // 1MHz Timer Frequency
        50,      // period is 50 clock cycles to set the PWM frequency to 20kHz
        NULL,
        {
                {PWM_OUTPUT_DISABLED, NULL}, // CH1 disabled
                {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Enable CH2
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
        },
        0,
        0
};


int main(void)
{
	chSysInit();
	halInit();
	sdStart(&SD2, NULL);
	// set A6 to PWM
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_ALTERNATE(2));
	// set A6 to input
	palSetPadMode(GPIOA, GPIOA_PIN6, PAL_MODE_OUTPUT_PUSHPULL);
	// set A4 high
	palSetPad(GPIOA, GPIOA_PIN6);
	// Start PWM driver on Timer 3
	pwmStart(&PWMD3, &pwmcfg);
	while(1) {
		// enable PWM on Timer 3 channel 1, duty cycle 50%
		pwmEnableChannel(&PWMD3, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 5000));
		chThdSleepMilliseconds(100);
	}
}


