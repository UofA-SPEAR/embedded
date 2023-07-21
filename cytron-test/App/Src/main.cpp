#include "ch.h"
#include "hal.h"
#include <stdlib.h>
#include <stdint.h>

#define FRAME_ID_VELOCITY 0x10

#define ACTUATOR1_ID 0x1
#define ACTUATOR2_ID 0x2

struct can_velocity {
	uint8_t actuator_id;
	float velocity;
};

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
QEIConfig qeicfg = {
	.mode = QEI_MODE_QUADRATURE,
	.resolution = QEI_BOTH_EDGES,
	.dirinv = QEI_DIRINV_FALSE,
	.overflow = QEI_OVERFLOW_WRAP,
	.min = 0,
	.max = 32767,
	.notify_cb = NULL,
	.overflow_cb = NULL
};



struct dc_motor_cfg {
	int PWM_channel;
	stm32_gpio_t* dir_gpio_port;
	int dir_gpio_pad;
	objects_fifo_t *fifo;
};

static objects_fifo_t actuator1_cmds;
static msg_t actuator1_msg_buffer[10];
static float actuator1_cmd_buffer[10];

static objects_fifo_t actuator2_cmds;
static msg_t actuator2_msg_buffer[10];
static float actuator2_cmd_buffer[10];


struct dc_motor_cfg motor_left_cfg = {
	.PWM_channel = 1,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 0,
	.fifo = &actuator1_cmds
};

struct dc_motor_cfg motor_right_cfg = {
	.PWM_channel = 3,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 2,
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
 * Derived from ChibiOS/ChibiOS/testhal/STM32/STM32F3xx/CAN/main.c
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
	txmsg.DLC = 2;
	
	while (true) {
		txmsg.data16[0] = qeiGetCountI(&QEID3);
		canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
		chThdSleepMilliseconds(500);
	}
}

void handle_velocity_cmd(struct can_velocity* msg)
{
	float *cmdObj;
	switch (msg->actuator_id) {
	case ACTUATOR1_ID:
		cmdObj = (float *)chFifoTakeObjectTimeout(&actuator1_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			*cmdObj = msg->velocity;
			chFifoSendObject(&actuator1_cmds, (void *)cmdObj);
		}
		break;
	case ACTUATOR2_ID:
		cmdObj = (float *)chFifoTakeObjectTimeout(&actuator2_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			*cmdObj = msg->velocity;
			chFifoSendObject(&actuator2_cmds, (void *)cmdObj);
		}
		break;
	}
}

void process_can_frame(CANRxFrame *rxmsg)
{
	// TODO: Refactor
	if (rxmsg->IDE == CAN_IDE_EXT) {
		switch ((rxmsg->EID >> 8) & 0xffff) {
		case FRAME_ID_VELOCITY:
			handle_velocity_cmd((struct can_velocity*)rxmsg->data8);
		}
	}
}

/*
 * Receiver thread.
 * Derived from ChibiOS/ChibiOS/testhal/STM32/STM32F3xx/CAN/main.c
 */
static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, p) {
	event_listener_t el;
	CANRxFrame rxmsg;

	(void)p;
	chRegSetThreadName("receiver");
	chEvtRegister(&CAND1.rxfull_event, &el, 0);
	while (true) {
		if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100)) == 0)
			continue;
		while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
			process_can_frame(&rxmsg);
		}
	}
	chEvtUnregister(&CAND1.rxfull_event, &el);
}

static THD_WORKING_AREA(actuator1_wa, 256);
static THD_WORKING_AREA(actuator2_wa, 256);
static THD_FUNCTION(actuator, arg)
{
	struct dc_motor_cfg *cfg = (struct dc_motor_cfg*)arg;
	float *recv_actuator_cmd;
	int curr_actuator_cmd;
	palSetPadMode(cfg->dir_gpio_port, cfg->dir_gpio_pad, PAL_MODE_OUTPUT_PUSHPULL);
	while (true) {
		msg_t status = chFifoReceiveObjectTimeout(cfg->fifo, (void**)&recv_actuator_cmd, TIME_US2I(100));
		if (status == MSG_OK) {
			curr_actuator_cmd = (int)(10000*(*recv_actuator_cmd));
			if (curr_actuator_cmd >= 0) {
				palSetPad(cfg->dir_gpio_port, cfg->dir_gpio_pad);
			} else {
				palClearPad(cfg->dir_gpio_port, cfg->dir_gpio_pad);
			}
			pwmEnableChannel(&PWMD2, cfg->PWM_channel, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, abs(curr_actuator_cmd)));
			chFifoReturnObject(cfg->fifo, recv_actuator_cmd);
		}
		chThdSleepMilliseconds(1);
	}
}


int main(void)
{
	chSysInit();
	halInit();

	canStart(&CAND1, &cancfg);
	
	// set A1 to PWM
	palSetPadMode(GPIOA, 1, PAL_MODE_ALTERNATE(1));
	// set A3 to PWM
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));
	// Start PWM driver on Timer 3
	pwmStart(&PWMD2, &pwmcfg);

	palSetPadMode(GPIOB, GPIOB_PIN4, PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_ALTERNATE(2));
	qeiStart(&QEID3, &qeicfg);
	qeiEnable(&QEID3);

	chFifoObjectInit(&actuator1_cmds, sizeof(float), 10, (void *)actuator1_cmd_buffer, actuator1_msg_buffer);
	chFifoObjectInit(&actuator2_cmds, sizeof(float), 10, (void *)actuator2_cmd_buffer, actuator2_msg_buffer);
	
	chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
	chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
	chThdCreateStatic(actuator1_wa, sizeof(actuator1_wa), NORMALPRIO + 7, actuator, (void *)&motor_left_cfg);
	chThdCreateStatic(actuator2_wa, sizeof(actuator2_wa), NORMALPRIO + 7, actuator, (void *)&motor_right_cfg);
	while(1) {
		chThdSleepMilliseconds(50);
	}
	return 0;
}
