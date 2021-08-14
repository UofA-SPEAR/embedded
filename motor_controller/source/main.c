#include "ch.h"
#include "hal.h"

#include "core_cm4.h"

#include "main.h"
#include "drv8701.h"
#include "settings.h"
#include "encoders.h"

#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/equipment/actuator/Status.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

// these bounds are needed, as not the potentiometers will not experience their full range
#define LOWER_POT_BOUND 0
#define UPPER_POT_BOUND 4096

#define BASE_NODE_ID 30

arm_pid_instance_f32 pid;
int32_t desired_position;

bool flag_motor_running = false;

thread_t *RunMotor_thread;
thread_t *Heartbeat_thread;

enum motor_reversal {
	MOTOR_BACKWARDS = -1,
	MOTOR_FORWARDS = 1,
};

static int motor_reversed = MOTOR_FORWARDS;
static bool flag_motor_enabled = false;

/// @brief Routine to keep motor in an angular position, according to config.
static void motor_run_angular(void)
{
	uavcan_equipment_actuator_Status status;
	static int32_t current_position = 0;
	uint8_t status_buf[20];
	float error, out;
	int32_t out_int;

	current_position = encoder_read();
	// Actually return as position, not just encoder value maybe?
	status.position = current_position;

	error = (float) (desired_position - current_position);
	error *= motor_reversed;

	// Run control loop
	out = arm_pid_f32(&pid, error);
	out_int = out * 1000;
	// Cap at 100% effort
	// TODO should be done in drv8701_set, but that should probably be don
	//      with more revamp effort
	if (out_int > 10000) out_int = 10000;
	else if (out_int < -10000) out_int = -10000;

	drv8701_set(out_int & 0xFFFF);

	// Send status info
	int len = uavcan_equipment_actuator_Status_encode(&status, &status_buf);
	canardBroadcast(&m_canard_instance,
		UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE,
		UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID,
		&inout_transfer_id,
		5,
		&status_buf,
		len);
}

/// @brief Runs motor without speed or position control, need to send raw
///        PWM values.
static void motor_run_ol(void)
{
	drv8701_set(desired_position);
}

/// Function pointer to swap out motor control strategies.
static void (*motor_run)(void);

/// @brief Enable/disable motor and decide on run strategy
void motor_init(void)
{
	// Default to angular
	// TODO maybe have a bit more sane way of checking settings.
	motor_run = motor_run_angular;

	if (run_settings[get_id_by_name("spear.motor.encoder.type")].value.integer == ENCODER_NONE)
		motor_run = motor_run_ol;

	if (run_settings[get_id_by_name("spear.motor.reversed")].value.boolean)
		motor_reversed = MOTOR_BACKWARDS;

	if (run_settings[get_id_by_name("spear.motor.enabled")].value.boolean)
		flag_motor_enabled = true;
}

/// @brief Set desired position based on configured strategy.
///
/// @note Sets a desired encoder value, uses configuration to determine
///       how to obtain that from a float.
void motor_set(float position)
{
		flag_motor_running = true;
        desired_position = encoder_get_position(position);
}

/// @brief Determine CAN node ID from physical switches.
uint8_t read_node_id(void)
{
	uint8_t node_id = BASE_NODE_ID;
	uint8_t tmp = 0;

	tmp  = palReadPad(GPIOB, 15) << 3;
	tmp |= palReadPad(GPIOA, 8) << 2;
	tmp |= palReadPad(GPIOA, 9) << 1;
	tmp |= palReadPad(GPIOA, 10);

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
	//bool error = false;

	/*
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
	*/

	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
}

/**
 * @brief Runs motor control code once every MOTOR_CONTROL_PERIOD ms.
 */
static THD_WORKING_AREA(RunMotorWorkingArea, 1024);
static THD_FUNCTION(RunMotor, arg)
{
	systime_t time;
    (void) arg;

    chRegSetThreadName("Run Motor");

    time = chVTGetSystemTimeX();

	while(true) {
		time += TIME_MS2I(MOTOR_CONTROL_PERIOD);

		// TODO find better way of shutting off
		if (flag_motor_running && flag_motor_enabled)
			motor_run();

		chThdSleepUntil(time);
    }
}

/// @brief Blinkies and CAN status
static THD_WORKING_AREA(HeartbeatWorkingArea, 512);
static THD_FUNCTION(Heartbeat, arg)
{
	systime_t time;
	(void) arg;
	chRegSetThreadName("Heartbeat");
	time = chVTGetSystemTimeX();

	while (true) {
		time += TIME_MS2I(1000);
		palToggleLine(LINE_LED);
		publish_nodeStatus();
		chThdSleepUntil(time);
	}
}

// To make a system reset, use NVIC_SystemReset()
int main(void) {
	uint8_t node_id = 0;

	halInit();
	chSysInit();

	node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	node_mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION;
	load_settings();
	for (int i = 0; i < NUM_SETTINGS; i++)
		run_settings[i].value = saved_settings[i].value;
	check_settings();
	motor_init();
	drv8701_init();
	encoder_init();
	drv8701_set_current(15u);
	comInit();
	node_id = read_node_id();
	canardSetLocalNodeID(&m_canard_instance, node_id);

	// setup PID
	memset(&pid, 0, sizeof(arm_pid_instance_f32));
	pid.Kp = run_settings[get_id_by_name("spear.motor.pid.Kp")].value.real;
	pid.Ki = run_settings[get_id_by_name("spear.motor.pid.Ki")].value.real;
	pid.Kd = run_settings[get_id_by_name("spear.motor.pid.Kd")].value.real;
	arm_pid_init_f32(&pid, 1);

	// TODO maybe handle errors here?
	RunMotor_thread = chThdCreateStatic(RunMotorWorkingArea,
			sizeof(RunMotorWorkingArea), HIGHPRIO, RunMotor, NULL);

	Heartbeat_thread = chThdCreateStatic(HeartbeatWorkingArea,
		sizeof(HeartbeatWorkingArea), NORMALPRIO, Heartbeat, NULL);

	chThdSetPriority(LOWPRIO);
	coms_handle_forever();
}

// Simply needs to be defined somewhere
int usleep(useconds_t usec)
{
	chThdSleepMicroseconds(usec);
	return 0;
}
