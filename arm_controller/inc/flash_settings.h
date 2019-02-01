/*
 * flash_settings.h
 *
 *  Created on: Jan 31, 2019
 *      Author: isthatme
 */

#ifndef FLASH_SETTINGS_H_
#define FLASH_SETTINGS_H_

#include "main.h"

typedef struct {
	uint8_t enabled;
	uint8_t actuator_id;
	float Kp;
	float Ki;
	float Kd;
	encoder_type_t encoder_type;
} motor_settings_t;

typedef struct {
	motor_settings_t motor1;
} flash_settings_t;

HAL_StatusTypeDef program_settings(void);

#endif /* FLASH_SETTINGS_H_ */
