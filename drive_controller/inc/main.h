/*
 * main.h
 *
 *  Created on: Feb 12, 2019
 *      Author: isthatme
 */

#ifndef MAIN_H_
#define MAIN_H_


#include "stm32f3xx.h"

#include "sabertooth.h"
#include "clocks.h"
#include "coms.h"
#include "encoders.h"

#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"

// Time (in ms) from last received message to stop motors
#define MOTOR_TIMEOUT_MS 1000

sabertooth_t saberA, saberB;

extern float motorA_speed, motorB_speed;

extern uint32_t motor_timeout;

extern bool motor_handle_flag;

#endif /* MAIN_H_ */
