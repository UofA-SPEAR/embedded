#include "ch.h"
#include "hal.h"
#include <stdlib.h>
#include <stdint.h>

#define FRAME_ID_VELOCITY 0x10
#define FRAME_ID_POSITION 0x11
#define FRAME_ID_PARAMETER 0x40

#define ACTUATOR1_ID 35
#define ACTUATOR2_ID 34

enum param_ids {
	STEP_PERIOD = 0,
	STEPS_PER_REV
};

struct can_velocity {
	uint8_t actuator_id;
	float velocity;
};

struct can_position {
	uint8_t actuator_id;
	float position;
};

struct can_parameter {
	uint8_t actuator_id;
	uint8_t write;
	uint8_t index;
	union param_val {
		uint32_t uint32_val;
		float float_val;
	} value;
};

enum ActuatorMode {
	MODE_VELOCITY,
	MODE_POSITION
};

struct actuator_cmd {
	float setpoint;
	enum ActuatorMode mode;
};

PWMConfig pwmcfg = {
        .frequency = 1000000,  // 1MHz Timer Frequency
        .period = 50,      // period is 50 clock cycles to set the PWM frequency to 20kHz
        .callback = NULL,
        .channels = {
                {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // CH1 disabled
                {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Enable CH2
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        },
        .cr2 = 0,
        .dier = 0
};

QEIConfig qeicfg = {
	.mode = QEI_MODE_QUADRATURE,
	.resolution = QEI_BOTH_EDGES,
	.dirinv = QEI_DIRINV_FALSE,
	.overflow = QEI_OVERFLOW_WRAP,
	.min = 0,
	.max = 2443,
	.notify_cb = NULL,
	.overflow_cb = NULL
};

struct dc_motor_cfg {
	int PWM_channel;
	stm32_gpio_t* dir_gpio_port;
	int dir_gpio_pad;
	float steps_per_revolution;
	int step_period_us;
	// In radians - Would degrees make more sense?
	float min_angle;
	float max_angle;
	objects_fifo_t *fifo;
};

static objects_fifo_t actuator1_cmds;
static msg_t actuator1_msg_buffer[10];
static struct actuator_cmd actuator1_cmd_buffer[10];

static objects_fifo_t actuator2_cmds;
static msg_t actuator2_msg_buffer[10];
static struct actuator_cmd actuator2_cmd_buffer[10];

mutex_t actuator1_cfg_mtx;
mutex_t actuator2_cfg_mtx;
struct dc_motor_cfg *actuator1_cfg;
struct dc_motor_cfg *actuator2_cfg;

struct dc_motor_cfg joint6_cfg = {
	.PWM_channel = 1,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 0,
	.steps_per_revolution = 200 * 32,
	.step_period_us = 25,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator1_cmds
};

struct dc_motor_cfg joint5_cfg = {
	.PWM_channel = 3,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 2,
	.steps_per_revolution = 200 * 8 * 50,
	.step_period_us = 25,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator2_cmds
};

struct dc_motor_cfg joint4_cfg = {
	.PWM_channel = 1,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 0,
	.steps_per_revolution = 200 * 8 * 80,
	.step_period_us = 25,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator1_cmds
};

struct dc_motor_cfg joint3_cfg = {
	.PWM_channel = 3,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 2,
	.steps_per_revolution = 200 * 8 * 80,
	.step_period_us = 25,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator2_cmds
};

struct dc_motor_cfg joint2_cfg = {
	.PWM_channel = 1,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 0,
	.steps_per_revolution = 200 * 200,
	.step_period_us = 1000,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator1_cmds
};

struct dc_motor_cfg joint1_cfg = {
	.PWM_channel = 3,
	.dir_gpio_port = GPIOA,
	.dir_gpio_pad = 2,
	.steps_per_revolution = 200 * 8 * 100,
	.step_period_us = 400,
	.min_angle = 0,
	.max_angle = 6.2831853,
	.fifo = &actuator2_cmds
};

static const CANConfig cancfg {
	/* Automatic recovery from Bus-Off,
	 * Transmit buffers operate in FIFO mode */
	.mcr = CAN_MCR_ABOM | CAN_MCR_TXFP,
	/* 1 Mbaud bit rate from 36 MHz APB1 clock, sample point 87.5% */
	.btr = CAN_BTR_SJW(0) | CAN_BTR_TS2(1) | CAN_BTR_TS1(14) | CAN_BTR_BRP(1)
};

// TODO: Move these back into the local storage of the motor thread
// These are currently global only for debugging purposes


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
	txmsg.RTR = CAN_RTR_DATA;
	txmsg.DLC = 4;

	while (true) {
		chThdSleepMilliseconds(1);
	}
}

void handle_velocity_cmd(struct can_velocity* msg)
{
	struct actuator_cmd *cmdObj;
	switch (msg->actuator_id) {
	case ACTUATOR1_ID:
		cmdObj = (struct actuator_cmd*)chFifoTakeObjectTimeout(&actuator1_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			cmdObj->setpoint = msg->velocity;
			cmdObj->mode = MODE_VELOCITY;
			chFifoSendObject(&actuator1_cmds, (void *)cmdObj);
		}
		break;
	case ACTUATOR2_ID:
		cmdObj = (struct actuator_cmd*)chFifoTakeObjectTimeout(&actuator2_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			cmdObj->setpoint = msg->velocity;
			cmdObj->mode = MODE_VELOCITY;
			chFifoSendObject(&actuator2_cmds, (void *)cmdObj);
		}
		break;
	}
}

void handle_position_cmd(struct can_position* msg)
{
	struct actuator_cmd *cmdObj;
	switch (msg->actuator_id) {
	case ACTUATOR1_ID:
		cmdObj = (struct actuator_cmd*)chFifoTakeObjectTimeout(&actuator1_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			cmdObj->setpoint = msg->position;
			cmdObj->mode = MODE_POSITION;
			chFifoSendObject(&actuator1_cmds, (void *)cmdObj);
		}
		break;
	case ACTUATOR2_ID:
		cmdObj = (struct actuator_cmd*)chFifoTakeObjectTimeout(&actuator2_cmds, TIME_US2I(100));
		if (cmdObj != NULL) {
			cmdObj->setpoint = msg->position;
			cmdObj->mode = MODE_POSITION;
			chFifoSendObject(&actuator2_cmds, (void *)cmdObj);
		}
		break;
	}
}

void set_param(struct dc_motor_cfg *cfg, struct can_parameter *msg)
{
	switch (msg->index) {
	case (STEP_PERIOD):
		cfg->step_period_us = msg->value.uint32_val;
		break;
	case (STEPS_PER_REV):
		cfg->steps_per_revolution = msg->value.uint32_val;
		break;
	}
}

void handle_parameter_cmd(struct can_parameter *msg)
{
	switch (msg->actuator_id) {
	case ACTUATOR1_ID:
		if (msg->write) {
			set_param(actuator1_cfg, msg);
		}
		break;
	case ACTUATOR2_ID:
		if (msg->write) {
			set_param(actuator2_cfg, msg);
		}
		break;
	}
}

void process_can_frame(CANRxFrame *rxmsg)
{
	// Our custom protocol only uses 29-bit (extended) CAN IDs
	if (rxmsg->IDE == CAN_IDE_EXT) {
		// Bits 23:8 of the CAN ID are the message identifier
		switch ((rxmsg->EID >> 8) & 0xffff) {
		case FRAME_ID_VELOCITY:
			handle_velocity_cmd((struct can_velocity*)rxmsg->data8);
			break;
		case FRAME_ID_POSITION:
			handle_position_cmd((struct can_position*)rxmsg->data8);
			break;
		case FRAME_ID_PARAMETER:
			handle_parameter_cmd((struct can_parameter*)rxmsg->data8);
			break;
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
	struct actuator_cmd *recv_actuator_cmd;
	int old_steps_per_rev = 0;
	float angle_scale_factor = 0.0;
	int counter = 0;
	int target = 0;
	enum ActuatorMode mode = MODE_POSITION;
	float setpoint = 0.0;
	int timeout;

	// Direction pin
	palSetPadMode(cfg->dir_gpio_port, cfg->dir_gpio_pad, PAL_MODE_OUTPUT_PUSHPULL);

	while (true) {
		if (cfg->steps_per_revolution != old_steps_per_rev) {
			old_steps_per_rev = cfg->steps_per_revolution;
			angle_scale_factor = cfg->steps_per_revolution / 6.283185307179586;
		}
		// Check for new encoder commands
		msg_t status = chFifoReceiveObjectTimeout(cfg->fifo, (void**)&recv_actuator_cmd, TIME_US2I(100));
		if (status == MSG_OK){
			setpoint = recv_actuator_cmd->setpoint;
			mode = recv_actuator_cmd->mode;
			chFifoReturnObject(cfg->fifo, recv_actuator_cmd);
		}

		if (mode == MODE_POSITION) {
			target = (int)(setpoint * angle_scale_factor);
			timeout = cfg->step_period_us/2;
		} else {
			if (setpoint > 0) {
				target++;
				timeout = (int)(cfg->step_period_us / (2 * setpoint));
			} else if (setpoint < 0) {
				target--;
				timeout = (int)(cfg->step_period_us / (-2 * setpoint));
			}
		}

		if (target == counter) {
			palClearPad(cfg->dir_gpio_port, cfg->PWM_channel);
		} else if (target > counter) {
			palSetPad(cfg->dir_gpio_port, cfg->dir_gpio_pad);
			// Pulse step pin
			palSetPad(cfg->dir_gpio_port, cfg->PWM_channel);
			chThdSleepMicroseconds(timeout);
			palClearPad(cfg->dir_gpio_port, cfg->PWM_channel);
			counter++;
		} else if (target < counter) {
			palClearPad(cfg->dir_gpio_port, cfg->dir_gpio_pad);
			// Pulse step pin
			palSetPad(cfg->dir_gpio_port, cfg->PWM_channel);
			chThdSleepMicroseconds(timeout);
			palClearPad(cfg->dir_gpio_port, cfg->PWM_channel);
			counter--;
		}
		chThdSleepMicroseconds(timeout);
	}
}

int main(void)
{
	chSysInit();
	halInit();

	canStart(&CAND1, &cancfg);

	// set A1 and A3 to GPIO for TB6600 step pin
	palSetPadMode(GPIOA, 1, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);

	// FIFO buffers for sending commands received by the CAN thread to the motor controller threads
	chFifoObjectInit(&actuator1_cmds, sizeof(float), 10, (void *)actuator1_cmd_buffer, actuator1_msg_buffer);
	chFifoObjectInit(&actuator2_cmds, sizeof(float), 10, (void *)actuator2_cmd_buffer, actuator2_msg_buffer);

	// CAN threads
	chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
	chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);

	chMtxObjectInit(&actuator1_cfg_mtx);
	chMtxObjectInit(&actuator2_cfg_mtx);
	actuator1_cfg = &joint6_cfg;
	actuator2_cfg = &joint5_cfg;
	// Motor control threads
	chThdCreateStatic(actuator1_wa, sizeof(actuator1_wa), NORMALPRIO + 7, actuator, (void *)actuator1_cfg);
	chThdCreateStatic(actuator2_wa, sizeof(actuator2_wa), NORMALPRIO + 7, actuator, (void *)actuator2_cfg);

	// Main (idle) loop
	while(1) {
		chThdSleepMilliseconds(50);
	}
	return 0;
}
