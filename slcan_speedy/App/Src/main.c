#include "ch.h"
#include "hal.h"
#include "utils.h"
#include "slcan_shell.h"
#include "string.h"
#include "board.h"

int main(void)
{
	// These are mandatory for the project
	halInit();
	chSysInit();
	sdStart(&SD2, NULL);
	canShellInit();
	while(1) {
		chThdSleepMilliseconds(1000);
	}
}

