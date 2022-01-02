/** This is a really basic example of using digital pins on the 
SPEAR SPEEDY such as output and polling input **/

#include "ch.h"
#include "hal.h"

int main(void)
{
	// These are mandatory for the project
	chSysInit();
	halInit();
	// Let's blink the onboard LED at a duration of 100ms
	while(1) {
		// This function is to toggle LED, if you want to turn on the LED, use palSetLine/palSetPad or 
		// turn off with palClearLine/palClearPad
		palToggleLine(LINE_LED); // this can be replace with palTogglePad(GPIOC, GPIOC_LED)
		// sleep at 100ms
		chThdSleepMilliseconds(100);
	}
}


