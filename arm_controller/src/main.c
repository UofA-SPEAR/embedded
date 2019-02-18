#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "core_cm4.h"

#include "main.h"
#include "vnh5019.h"
#include "encoders.h"
#include "clocks.h"
#include "coms.h"
#include "flash_settings.h"

#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"

#include "uavcan/protocol/NodeStatus.h"

// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096

#define BASE_NODE_ID 30

vnh5019_t motorA, motorB;
arm_pid_instance_f32 pidA, pidB;

uint32_t motorA_desired_position;
uint32_t motorB_desired_position;



void setup(){
	HAL_Init();

	// init clock
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	SystemCoreClockUpdate();
	SystemClock_Config();
}

// Run PID and motor control
void run_motorA() {
	int16_t out_int;

	// Check if the motor is even enabled
	if (run_settings.motor[0].enabled) {
		// Check if encoder is potentiometer type.
		if (run_settings.motor[0].encoder.type == ENCODER_POTENTIOMETER) {
			uint32_t current_position = potA_read();

			float error = current_position - motorA_desired_position;

			float out = arm_pid_f32(&pidA, error);
			out_int = out * 1000;

			vnh5019_set(&motorA, out_int);
		}
	}
}

void motor_init() {
	motorA.digital.in_a = GPIO_PIN_4;
	motorA.digital.in_b = GPIO_PIN_3;
	motorA.digital.en_a = GPIO_PIN_6;
	motorA.digital.en_b = GPIO_PIN_5;
	motorA.digital.port = GPIOB;

	motorA.pwm.pin 		= GPIO_PIN_7;
	motorA.pwm.port 	= GPIOB;
	motorA.pwm.tim_af	= GPIO_AF2_TIM4;
	motorA.pwm.tim_ch = TIM_CHANNEL_2;

	vnh5019_init(&motorA);


	/* Insanely large number, so no motor checks should happen
	 * until a position is received.
	 */
	last_runA = INT32_MAX;
	last_runB = INT32_MAX;
}

uint8_t read_node_id(void) {
	// Enable GPIO clock
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14 |
			GPIO_PIN_13 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	uint8_t node_id = BASE_NODE_ID;
	uint8_t tmp = 0;

	tmp =  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) << 3;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) << 2;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) << 1;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

	node_id += tmp;
	return node_id;
}

/** @brief Checks if run settings are valid.
 *
 * If settings are invalid, it will disable the motors and set the nodestatus
 * appropriately
 */
void check_settings(void) {
	bool error;

	// Make sure checks run for both motors
	for (uint8_t i = 0; i < 2; i++) {
		// Only run checks if the motor is even enabled
		if (run_settings.motor[i].enabled) {
			// Encoder should have more than 0 range of motion
			if (run_settings.motor[i].encoder.min >= run_settings.motor[0].encoder.max) {
				error = true;
			}

			// Checking for radial settings
			if (run_settings.motor[i].encoder.to_radians != 0) {
				// Linear settings should not be set
				if (run_settings.motor[i].linear.support_length != 0) {
					error = true;
				}
			}

			// Checking for linear settings
			if (run_settings.motor[i].linear.support_length != 0) {
				// All values should be more than 0
				if ((run_settings.motor[i].linear.support_length <= 0) ||
						(run_settings.motor[i].linear.arm_length <= 0) ||
						(run_settings.motor[i].linear.length_min <= 0) ||
						(run_settings.motor[i].linear.length_max <= 0)) {
					error = true;
				}

				// Extends positively
				if ((run_settings.motor[i].linear.length_min >=
						run_settings.motor[i].linear.length_max)) {
					error = true;
				}

				// Radial settings should not be set
				if (run_settings.motor[i].encoder.to_radians != 0) {
					error = true;
				}
			}
		}
	}

	// On configuration error, disable motors and change NodeStatus
	if (error) {
		run_settings.motor[0].enabled = 0;
		run_settings.motor[1].enabled = 0;
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR;
	} else {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	}

	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
}

// To make a system reset, use NVIC_SystemReset()
int main(void) {
	uint8_t node_id = 0;

	node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION;

	setup();

	load_settings();
	run_settings = saved_settings;
	check_settings();
	motor_init();
	potA_init();

	comInit();
	node_id = read_node_id();
	canardSetLocalNodeID(&m_canard_instance, node_id);

	// setup PID
	memset(&pidA, 0, sizeof(arm_pid_instance_f32));
	pidA.Kp = run_settings.motor[0].pid.Kp;
	pidA.Ki = run_settings.motor[0].pid.Ki;
	pidA.Kd = run_settings.motor[0].pid.Kd;
	arm_pid_init_f32(&pidA, 1);


	for (;;) {

		if ((HAL_GetTick() - last_runA >= 100) && run_settings.motor[0].enabled) {
			run_motorA();
			last_runA = HAL_GetTick();
			canardCleanupStaleTransfers(&m_canard_instance, can_timestamp_usec);
		}

		publish_nodeStatus();

		tx_once();
		handle_frame();
	}
}


