/** This is a really basic example of using digital pins on the 
SPEAR SPEEDY such as output and polling input **/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
int main(void)
{
	// These are mandatory for the project
	chSysInit();
	halInit();
	sdStart(&SD2, NULL);
	// adcStart(&ADCD1, NULL);
	// Let's blink the onboard LED at a duration of 100ms
	while(1) {
		// start ADC...

		chprintf((BaseSequentialStream*)&SD2, "Hello World \r\n");
		// This function is to toggle LED, if you want to turn on the LED, use palSetLine/palSetPad or 
		// turn off with palClearLine/palClearPad
		palToggleLine(LINE_LED); // this can be replace with palTogglePad(GPIOC, GPIOC_LED)
		// sleep at 100ms
		chThdSleepMilliseconds(1000);
	}
}


