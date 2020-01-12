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

vnh5019_t motor;
arm_pid_instance_f32 pid[2];
int32_t desired_position;

bool flag_motor_running = false;



void setup(){
	HAL_Init();

	SystemClock_Config();
}

// Run PID and motor control
void run_motor(void) {
	int16_t out_int;
	static int32_t current_position = 0;

	uavcan_equipment_actuator_Status status;

	// Check if the motor is even enabled
	if (run_settings.motor.enabled) {
		// Check if encoder is potentiometer type.
		if (run_settings.motor.encoder.type == ENCODER_POTENTIOMETER) {
			// Read potentiometer position
			current_position = pot_read(0);
		} else if (run_settings.motor.encoder.type == ENCODER_QUADRATURE) {
			// Read current encoder position. We don't care about wraps at the moment
			current_position = (TIM8->CNT) - ENCODER_START_VAL;
		} else if (run_settings.motor.encoder.type == ENCODER_ABSOLUTE_DIGITAL) {
			int32_t tmp_position;
			if ((tmp_position = ems22_read_position(0)) != -1) {
				current_position = tmp_position;
			}
		} else {
			// should never get here
			status.position = 0;
			return;
		}

		// will be wrong in the linear case
		if (run_settings.motor.linear.support_length != 0.0) {
			status.position = current_position;
		} else {
			status.position = (float) current_position * run_settings.motor.encoder.to_radians;
		}

		float error;
		if (run_settings.motor.reversed) {
			// Reverse the error.
			// TODO evaluate if I should reverse the error or the motor output
			error = (float) - (desired_position - current_position);
		} else {
			error = (float) desired_position - current_position;
		}

		if (run_settings.motor.encoder.to_radians != (float)  0.0) {
			float out = arm_pid_f32(&pid, error);
			out_int = out * 1000;
		} else if (run_settings.motor.linear.support_length >= 0) {
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

		vnh5019_set(&motor, out_int);


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
	motor.digital.in_a.pin =    MOTOR_INA_PIN;
	motor.digital.in_a.port =   MOTOR_INA_PORT;
	motor.digital.in_b.pin =    MOTOR_INB_PIN;
	motor.digital.in_b.port =   MOTOR_INB_PORT;
	motor.digital.en_a.pin =    MOTOR_ENA_PIN;
	motor.digital.en_a.port =   MOTOR_ENA_PORT;
	motor.digital.en_b.pin =    MOTOR_ENB_PIN;
	motor.digital.en_b.port =   MOTOR_ENB_PORT;

	motor.pwm.pin =             MOTOR_PWM_PIN;
	motor.pwm.port =            MOTOR_PWM_PORT;
	motor.pwm.tim_instance =    MOTOR_PWM_TIM_INSTANCE;
	motor.pwm.tim_af =          MOTOR_PWM_TIM_AF;
	motor.pwm.tim_ch =          MOTOR_PWM_TIM_CHANNEL;

	vnh5019_init(&motor);
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
void check_settings(void)
{
	bool error = false;

    // Only run checks if the motor is even enabled
    if (run_settings.motor.enabled) {
        // Encoder should have more than 0 range of motion
        if (run_settings.motor.encoder.min >= run_settings.motor.encoder.max) {
            error = true;
        }

        // Checking for radial settings
        if (run_settings.motor.encoder.to_radians != 0) {
            // Linear settings should not be set
            if (run_settings.motor.linear.support_length != 0) {
                error = true;
            }
        }

        // Checking for linear settings
        if (run_settings.motor.linear.support_length != 0) {
            // All values should be more than 0
            if ((run_settings.motor.linear.support_length <= 0) ||
                    (run_settings.motor.linear.arm_length <= 0) ||
                    (run_settings.motor.linear.length_min <= 0) ||
                    (run_settings.motor.linear.length_max <= 0)) {
                error = true;
            }

            // Extends positively
            if ((run_settings.motor.linear.length_min >=
                    run_settings.motor.linear.length_max)) {
                error = true;
            }

            // Radial settings should not be set
            if (run_settings.motor.encoder.to_radians != 0) {
                error = true;
            }
        }
    }

	// On configuration error, disable motors and change NodeStatus
	if (error) {
		run_settings.motor.enabled = 0;
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR;
	} else {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	}

	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
}

/**
 * @brief Runs motor control code once every MOTOR_CONTROL_PERIOD ms.
 */
static THD_WORKING_AREA(RunMotorWorkingArea, 128);
static THD_FUNCTION(RunMotor, arg)
{
    (void) arg;
    chRegSetThreadName("Run Motor");

    systime_t time = chVTGetSystemTimeX();

    while(true) {
        time += M2ST(MOTOR_CONTROL_PERIOD);
        run_motor(0);
        chThdSleepUntil(time);
    }
}


// To make a system reset, use NVIC_SystemReset()
int main(void) {

    halInit();
    chSYsInit();


	uint8_t node_id = false;
    bool motor_run_thread_started = false;

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
        // TODO fix this logic up, maybe handle it inside the thread?
        if (flag_motor_running && !motor_run_thread_started) {
            thread_t* thd = chThdCreateStatic(RunMotorWorkingArea,
                    sizeof(RunMotorWorkingArea), HIGHPRIO, RunMotor, NULL);

            if (thd != NULL) {
                motor_run_thread_started = true;
            } else {
                // Abort!
                return 0;
            }
        }


		publish_nodeStatus();

		tx_once();
		handle_frame();
	}
}


