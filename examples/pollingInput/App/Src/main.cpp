/** This is a really basic example of using digital pins on the 
SPEAR SPEEDY such as polling input **/


// From the Blinky demo, now we know how to blink an LED, let's use a switch to turn on and off the LED.
// Connect Pin B0 from the SPEEDY to a switch with breadboard, the other side of the switch will be connected
// to GND on the SPEEDY

#include "ch.h"
#include "hal.h"

int main(void)
{
	// These are mandatory for the project
	chSysInit();
	halInit();
	// Configure B0 as Input Pullup
	palSetPadMode(GPIOB, GPIOB_PIN0, PAL_MODE_INPUT_PULLUP);
	while(1) {
		// read the digital input value
		uint8_t input = palReadPad(GPIOB, GPIOB_PIN0);
		if(input == 1) { // if the input is read as HIGH, turn on the LED
			palSetLine(LINE_LED);
		}
		else { // else turn off the LED
			palClearLine(LINE_LED);
		}
		// 1ms sleep for buffering....
		chThdSleep(1);
	}
}


