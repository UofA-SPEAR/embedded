#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "core_cm4.h"

#include "main.h"

#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/equipment/actuator/Status.h"

// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096

#define BASE_NODE_ID 30

// "dead" zone so we aren't oscillating
#define LINEAR_ACTUATOR_DEADZONE	350
#define LINEAR_ACTUATOR_POWER		500

vnh5019_t motors[2];
arm_pid_instance_f32 pid[2];
int32_t desired_positions[2];



void setup(){
	HAL_Init();

	SystemClock_Config();
}

// Run PID and motor control
void run_motor(uint8_t motor) {
	int16_t out_int;
	static int32_t current_position[2] = {0, 0};

	uavcan_equipment_actuator_Status status;

	// Check if the motor is even enabled
	if (run_settings.motor[motor].enabled) {
		// Check if encoder is potentiometer type.
		if (run_settings.motor[motor].encoder.type == ENCODER_POTENTIOMETER) {
			// Read potentiometer position
			current_position[motor] = pot_read(motor);
		} else if (run_settings.motor[motor].encoder.type == ENCODER_QUADRATURE) {
			// Read current encoder position. We don't care about wraps at the moment
			current_position[motor] = (TIM8->CNT) - ENCODER_START_VAL;
		} else if (run_settings.motor[motor].encoder.type == ENCODER_ABSOLUTE_DIGITAL) {
			int32_t tmp_position;
			if ((tmp_position = ems22_read_position(motor)) != -1) {
				current_position[motor] = tmp_position;
			}
		} else {
			// should never get here
			status.position = 0;
			return;
		}

		// will be wrong in the linear case
		if (run_settings.motor[motor].linear.support_length != 0.0) {
			status.position = current_position[motor];
		} else {
			status.position = (float) current_position[motor] * run_settings.motor[motor].encoder.to_radians;
		}

		float error;
		if (run_settings.motor[motor].reversed) {
			// Reverse the error.
			// TODO evaluate if I should reverse the error or the motor output
			error = (float) - (desired_positions[motor] - current_position[motor]);
		} else {
			error = (float) desired_positions[motor] - current_position[motor];
		}

		if (run_settings.motor[motor].encoder.to_radians != (float)  0.0) {
			float out = arm_pid_f32(&pid[motor], error);
			out_int = out * 1000;
		} else if (run_settings.motor[motor].linear.support_length >= 0) {
			// constant motor power, instead of PID, simpler

			if (error > LINEAR_ACTUATOR_DEADZONE) {
				out_int = LINEAR_ACTUATOR_POWER;
			} else if (error > (LINEAR_ACTUATOR_DEADZONE / 2)) {
				out_int = LINEAR_ACTUATOR_POWER / 2;
			} else if (error < -LINEAR_ACTUATOR_DEADZONE) {
				out_int = -LINEAR_ACTUATOR_POWER;
			} else if (error < -(LINEAR_ACTUATOR_DEADZONE / 2)) {
				out_int = -LINEAR_ACTUATOR_POWER / 2;
			} else {
				out_int = 0;
			}
		} else {
			// Should never get here
			return;
		}

		vnh5019_set(&motors[motor], out_int);


		// Send status info
		uint8_t status_buf[20];
		int len = uavcan_equipment_actuator_Status_encode(&status, &status_buf);

		canardBroadcast(&m_canard_instance,
			UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE,
			UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID,
			&inout_transfer_id,
			5,
			&status_buf,
			len);
	}
}

void motor_init() {
	motors[0].digital.in_a.pin = MOTORA_INA_PIN;
	motors[0].digital.in_a.port = MOTORA_INA_PORT;
	motors[0].digital.in_b.pin = MOTORA_INB_PIN;
	motors[0].digital.in_b.port = MOTORA_INB_PORT;
	motors[0].digital.en_a.pin = MOTORA_ENA_PIN;
	motors[0].digital.en_a.port = MOTORA_ENA_PORT;
	motors[0].digital.en_b.pin = MOTORA_ENB_PIN;
	motors[0].digital.en_b.port = MOTORA_ENB_PORT;

	motors[0].pwm.pin 		= MOTORA_PWM_PIN;
	motors[0].pwm.port 		= MOTORA_PWM_PORT;
	motors[0].pwm.tim_instance = MOTORA_PWM_TIM_INSTANCE;
	motors[0].pwm.tim_af	= MOTORA_PWM_TIM_AF;
	motors[0].pwm.tim_ch 	= MOTORA_PWM_TIM_CHANNEL;

	vnh5019_init(&motors[0]);

	motors[1].digital.in_a.pin = MOTORB_INA_PIN;
	motors[1].digital.in_a.port = MOTORB_INA_PORT;
	motors[1].digital.in_b.pin = MOTORB_INB_PIN;
	motors[1].digital.in_b.port = MOTORB_INB_PORT;
	motors[1].digital.en_a.pin = MOTORB_ENA_PIN;
	motors[1].digital.en_a.port = MOTORB_ENA_PORT;
	motors[1].digital.en_b.pin = MOTORB_ENB_PIN;
	motors[1].digital.en_b.port = MOTORB_ENB_PORT;

	motors[1].pwm.pin 		= MOTORB_PWM_PIN;
	motors[1].pwm.port 		= MOTORB_PWM_PORT;
	motors[1].pwm.tim_instance = MOTORB_PWM_TIM_INSTANCE;
	motors[1].pwm.tim_af	= MOTORB_PWM_TIM_AF;
	motors[1].pwm.tim_ch 	= MOTORB_PWM_TIM_CHANNEL;

	vnh5019_init(&motors[1]);


	/* Insanely large number, so no motor checks should happen
	 * until a position is received.
	 */
	last_run_times[0] = INT16_MAX;
	last_run_times[1] = INT16_MAX;
}

uint8_t read_node_id(void) {
	// Enable GPIO clock
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 |
			GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	uint8_t node_id = BASE_NODE_ID;
	uint8_t tmp = 0;

	tmp =  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) << 3;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) << 2;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) << 1;
	tmp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);

	node_id += tmp;
	return node_id;
}

/** @brief Checks if run settings are valid.
 *
 * If settings are invalid, it will disable the motors and set the nodestatus
 * appropriately
 */
void check_settings(void) {
	bool error = false;

	// Make sure checks run for both motors
	for (uint8_t i = 0; i < 2; i++) {
		// Only run checks if the motor is even enabled
		if (run_settings.motor[i].enabled) {
			// Encoder should have more than 0 range of motion
			if (run_settings.motor[i].encoder.min >= run_settings.motor[i].encoder.max) {
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
	uint8_t node_id = false;

	node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION;

	setup();

	load_settings();
	run_settings = saved_settings;
	check_settings();
	motor_init();

	// Initialize feedback
	for (uint8_t i = 0; i < 2; i++) {
		switch (run_settings.motor[i].encoder.type) {
		case (ENCODER_POTENTIOMETER):
				pot_init(0);
				break;
		case (ENCODER_QUADRATURE):
				encoder_init();
				break;
		case (ENCODER_ABSOLUTE_DIGITAL):
				ems22_init();
				break;
		default:
				// do nothing
				break;
		}
	}

	comInit();
	node_id = read_node_id();
	canardSetLocalNodeID(&m_canard_instance, node_id);

	// setup PID
	memset(&pid[0], 0, sizeof(arm_pid_instance_f32));
	pid[0].Kp = run_settings.motor[0].pid.Kp;
	pid[0].Ki = run_settings.motor[0].pid.Ki;
	pid[0].Kd = run_settings.motor[0].pid.Kd;
	arm_pid_init_f32(&pid[0], 1);

	memset(&pid[1], 0, sizeof(arm_pid_instance_f32));
	pid[1].Kp = run_settings.motor[1].pid.Kp;
	pid[1].Ki = run_settings.motor[1].pid.Ki;
	pid[1].Kd = run_settings.motor[1].pid.Kd;
	arm_pid_init_f32(&pid[1], 1);


	for (;;) {

		if ((HAL_GetTick() - last_run_times[0] >= 100) && run_settings.motor[0].enabled
				&& (last_run_times[0] != INT16_MAX)) {
			run_motor(0);
			last_run_times[0] = HAL_GetTick();
			canardCleanupStaleTransfers(&m_canard_instance, can_timestamp_usec);
		}
		if ((HAL_GetTick() - last_run_times[1] >= 100) && run_settings.motor[1].enabled
				&& (last_run_times[1] != INT16_MAX)) {
			run_motor(1);
			last_run_times[1] = HAL_GetTick();
			canardCleanupStaleTransfers(&m_canard_instance, can_timestamp_usec);
		}

		publish_nodeStatus();

		tx_once();
		handle_frame();
	}
}


