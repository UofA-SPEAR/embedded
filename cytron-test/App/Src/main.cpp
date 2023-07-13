#include "ch.h"
#include "hal.h"

PWMConfig pwmcfg = {
        1000000,  // 1MHz Timer Frequency
        50,      // period is 50 clock cycles to set the PWM frequency to 20kHz
        NULL,
        {
                {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // CH1 disabled
                {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Enable CH2
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        },
        0,
        0
};

static const CANConfig cancfg {
	/* Automatic recovery from Bus-Off, 
	 * Transmit buffers operate in FIFO mode */
	.mcr = CAN_MCR_ABOM | CAN_MCR_TXFP,
	/* 1 Mbaud bit rate from 36 MHz APB1 clock, sample point 87.5% */
	.btr = CAN_BTR_SJW(0) | CAN_BTR_TS2(1) | CAN_BTR_TS1(14) | CAN_BTR_BRP(1)
};

/*
static objects_fifo_t actuator1_cmds;
static objects_fifo_t actuator2_cmds;
static msg_t actuator1_msg
*/

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
	
	// set A1 to PWM
	palSetPadMode(GPIOA, GPIOA_PIN1, PAL_MODE_ALTERNATE(1));
	// set A3 to PWM
	palSetPadMode(GPIOA, GPIOA_USART2_RX, PAL_MODE_ALTERNATE(1));
	// Start PWM driver on Timer 3
	pwmStart(&PWMD2, &pwmcfg);
	
	chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
	int i = 0;
	int dir = -100;
	while(1) {
		// enable PWM on Timer 3 channel 1, duty cycle 50%
		pwmEnableChannel(&PWMD2, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, i));
		//pwmEnableChannel(&PWMD2, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 9999-i));
		if (i == 0 || i == 10000) {
			dir *= -1;	
		}
		i += dir;
		chThdSleepMilliseconds(50);
	}
	return 0;
}
