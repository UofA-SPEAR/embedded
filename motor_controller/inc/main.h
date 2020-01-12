/**
 * @file main.h
 *
 *  Created on: Jan 25, 2019
 *      Author: isthatme
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f3xx.h"
#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"

#include "vnh5019.h"
#include "encoders.h"
#include "clocks.h"
#include "coms.h"
#include "flash_settings.h"
#include "ems22.h"

#define ROUND_UP(dividend, divisor) ((dividend + (divisor - 1)) / divisor)

/* ---- Motor A (0) Config ---- */
#define MOTOR_PWM_PIN 	        GPIO_PIN_8
#define MOTOR_PWM_PORT          GPIOC
#define MOTOR_PWM_TIM_INSTANCE  TIM8
#define MOTOR_PWM_TIM_AF        GPIO_AF4_TIM8
#define MOTOR_PWM_TIM_CHANNEL   TIM_CHANNEL_3

#define MOTOR_INA_PIN 	        GPIO_PIN_10
#define MOTOR_INA_PORT          GPIOA
#define MOTOR_INB_PIN 	        GPIO_PIN_9
#define MOTOR_INB_PORT          GPIOA
#define MOTOR_ENA_PIN 	        GPIO_PIN_7
#define MOTOR_ENA_PORT          GPIOC
#define MOTOR_ENB_PIN 	        GPIO_PIN_6
#define MOTOR_ENB_PORT	        GPIOC


/** Period of motor control loop */
#define MOTOR_CONTROL_PERIOD    100

extern int32_t desired_positions[2];
extern arm_pid_instance_f32 pid[2];

extern bool flag_motor_running;


int32_t last_run_times[2];


#endif /* MAIN_H_ */
