#include "ch.h"
#include "hal.h"
/*
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
};*/

static const CANConfig cancfg {
	/* Automatic recovery from Bus-Off, 
	 * Transmit buffers operate in FIFO mode */
	.mcr = CAN_MCR_ABOM | CAN_MCR_TXFP,
	/* 1 Mbaud bit rate from 36 MHz APB1 clock, sample point 87.5% */
	.btr = CAN_BTR_SJW(0) | CAN_BTR_TS2(1) | CAN_BTR_TS1(14) | CAN_BTR_BRP(1)
};

/*
 * Transmitter thread.
 */
static THD_WORKING_AREA(can_tx_wa, 256);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (true) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
    chThdSleepMilliseconds(500);
  }
}


int main(void)
{
	chSysInit();
	halInit();

	sdStart(&SD2, NULL);
	canStart(&CAND1, &cancfg);
	/*
	// set A6 to PWM
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_ALTERNATE(2));
	// set A6 to input
	palSetPadMode(GPIOA, GPIOA_PIN6, PAL_MODE_OUTPUT_PUSHPULL);
	// set A4 high
	palSetPad(GPIOA, GPIOA_PIN6);
	// Start PWM driver on Timer 3
	pwmStart(&PWMD3, &pwmcfg);
	*/
	chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
	while(1) {
		// enable PWM on Timer 3 channel 1, duty cycle 50%
		// pwmEnableChannel(&PWMD3, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 5000));
		chThdSleepMilliseconds(100);
	}
	return 0;
}
