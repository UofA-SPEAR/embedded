/*
 * flash_settings.h
 *
 *  Created on: Jan 31, 2019
 *      Author: isthatme
 */

#ifndef FLASH_SETTINGS_H_
#define FLASH_SETTINGS_H_

#include "main.h"
#include "encoders.h"

typedef enum {
	ENDSTOP_DISABLED,
	ENDSTOP_ENABLED,
	ENDSTOP_ENABLED_INVERTED,
} endstop_t;

typedef struct {
	uint8_t enabled;					//< Will the motor be used. NOT if the motor is physically enabled right now.
	uint8_t actuator_id;				//< UAVCAN actuator ID
	uint8_t reversed;					//< Is the motor reversed?
	uint8_t continuous;					//< Can the motor be run continuously?

	// PID settings
	struct {
		float Kp;						//< Kp tuning constant.
		float Ki;						//< Ki tuning constant.
		float Kd;						//< Kd tuning constant.
	} pid;

	// Encoder/positioning settings
	struct {
		encoder_type_t type;	//< Encoder type
		int64_t min;			//< Minimum bound of acceptable encoder value
		int64_t max;			//< Maximum bound of acceptable encoder value
		/*
		 * Make to_radian negative to invert the encoder "direction"
		 *
		 * if (to_radian > 0) {
		 * 		0 radians = encoder_min
		 * 		max_radians = encoder_max
		 * } else if (to_radian < 0) {
		 * 		0 radians = encoder_max
		 * 		max_radians = encoder_min
		 * }
		 */
		float to_radians;		//< Constant to convert encoder values to radians
		endstop_t endstop_min;
		endstop_t endstop_max;
	} encoder;

	/* Settings for using a linear actuator
	 * Assumes you are running the specific setup we have
	 */
	struct {
		float support_length; // Length of the support beam holding the linear actuator
		float arm_length; // Length of the "upper arm", from the base pivot to the linear actuator pin
		float length_min; // Fully retracted length of actuator
		float length_max; // Fully extended length of actuator
	} linear;
} motor_settings_t;

typedef struct {
	uint8_t boot;						//< Check to see if this is the first boot
	motor_settings_t motor[2];
} flash_settings_t;

extern flash_settings_t saved_settings;
extern flash_settings_t current_settings;
// Settings to be used for actually running the motor
extern flash_settings_t run_settings;

/** @brief Loads in user settings from flash.
 *
 * @note also checks if this is the firstboot, and sets decent default values
 */
void load_settings(void);

/** @brief writes current user settings into flash memory.
 *
 */
HAL_StatusTypeDef program_settings(void);

#endif /* FLASH_SETTINGS_H_ */
