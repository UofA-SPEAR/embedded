#include "ch.h"
#include "hal.h"

int main(void)
{
	chSysInit();
	halInit();
	sdStart(&SD2, NULL);
	while(1) {

		chThdSleepMilliseconds(100);
	}
}


