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

struct dc_motor_cfg {
	int PWM_channel;
	stm32_gpio_t* dir_gpio_port;
	int dir_gpio_pad;
	objects_fifo_t *fifo;
};

static objects_fifo_t actuator1_cmds;
static msg_t actuator1_msg_buffer[10];
static int actuator1_cmd_buffer[10];

static objects_fifo_t actuator2_cmds;
static msg_t actuator2_msg_buffer[10];
static int actuator2_cmd_buffer[10];


struct dc_motor_cfg motor_left_cfg = {
	.PWM_channel = 1,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = GPIOA_PIN0,
	.fifo = &actuator1_cmds
};

struct dc_motor_cfg motor_right_cfg = {
	.PWM_channel = 3,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = GPIOA_USART2_TX,
	.fifo = &actuator2_cmds
};

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
static THD_FUNCTION(can_tx, arg)
{
	CANTxFrame txmsg;

	(void)arg;
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

static THD_WORKING_AREA(actuator_wa, 256);
static THD_FUNCTION(actuator, arg)
{
	struct dc_motor_cfg *cfg = (struct dc_motor_cfg*)arg;
	int *recv_actuator_cmd;
	int curr_actuator_cmd;
	palSetPadMode(cfg->dir_gpio_port, cfg->dir_gpio_pad, PAL_MODE_OUTPUT_PUSHPULL);
	while (true) {
		msg_t status = chFifoReceiveObjectTimeout(cfg->fifo, (void**)&recv_actuator_cmd, TIME_US2I(100));
		if (status == MSG_OK) {
			curr_actuator_cmd = *recv_actuator_cmd;
			palSetPad(cfg->dir_gpio_port, cfg->dir_gpio_pad);
			pwmEnableChannel(&PWMD2, cfg->PWM_channel, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, curr_actuator_cmd));
			chFifoReturnObject(cfg->fifo, recv_actuator_cmd);
		}
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

	chFifoObjectInit(&actuator1_cmds, sizeof(int), 10, (void *)actuator1_cmd_buffer, actuator1_msg_buffer);
	chFifoObjectInit(&actuator2_cmds, sizeof(int), 10, (void *)actuator2_cmd_buffer, actuator2_msg_buffer);
	
	chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
	chThdCreateStatic(actuator_wa, sizeof(actuator_wa), NORMALPRIO + 7, actuator, (void *)&motor_left_cfg);
	int i = 0;
	int dir = -100;
	int *cmdObj;
	while(1) {
		cmdObj = (int *)chFifoTakeObjectTimeout(&actuator1_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			*cmdObj = i;
			chFifoSendObject(&actuator1_cmds, (void *)cmdObj);
		}
		if (i <= 0 || i >= 10000) {
			dir *= -1;
		}
		i += dir;
		chThdSleepMilliseconds(50);
	}
	return 0;
}
