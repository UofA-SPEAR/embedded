/** This is a really basic example of using digital pins on the 
SPEAR SPEEDY such as polling input **/


// Polling can be great since it is simple to use, however, it is not ideal since it requires CPU to constantly 
// monitoring the pin, Interrupt is better since it requires less CPU attention so that the CPU can do other tasks :))
// This demo will show that when you press a button, it will send "Button Pressed" while the LED is toggling in 100ms
// On Windows, use any terminal (Mobaxterm, Putty, ...) and configure it to the right COM Port and use baudrate of 115200

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

int main(void)
{
	// These are mandatory for the project
	chSysInit();
	halInit();

	// Starting the Serial Port that connected to your PC
	sdStart(&SD2, NULL);
	
	// Configure B0 as Input Pullup
	palSetPadMode(GPIOB, GPIOB_PIN0, PAL_MODE_INPUT_PULLUP);
	
	// Enable PAL Interrupt event Falling Edge so it doesn't really screw up. In application, it depends on you to define
	// when to use the interrupt(falling edge, rising edge or both)
	palEnablePadEvent(GPIOB, GPIOB_PIN0, PAL_EVENT_MODE_FALLING_EDGE);
	while(1) {
	
		// Wait for button with a timeout 
		msg_t result = palWaitPadTimeout(GPIOB, GPIOB_PIN0, 100);
		// Check the result
		if(result) {
			// if the button is pressed
			chprintf((BaseSequentialStream*)&SD2, "Button Pressed \r\n");
		}
		else {
			palToggleLine(LINE_LED);
		}		
	}
}


