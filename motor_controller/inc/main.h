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
#define MOTORA_PWM_PIN 	        GPIO_PIN_8
#define MOTORA_PWM_PORT         GPIOC
#define MOTORA_PWM_TIM_INSTANCE TIM8
#define MOTORA_PWM_TIM_AF       GPIO_AF4_TIM8
#define MOTORA_PWM_TIM_CHANNEL  TIM_CHANNEL_3

#define MOTORA_INA_PIN 	        GPIO_PIN_10
#define MOTORA_INA_PORT         GPIOA
#define MOTORA_INB_PIN 	        GPIO_PIN_9
#define MOTORA_INB_PORT         GPIOA
#define MOTORA_ENA_PIN 	        GPIO_PIN_7
#define MOTORA_ENA_PORT         GPIOC
#define MOTORA_ENB_PIN 	        GPIO_PIN_6
#define MOTORA_ENB_PORT	        GPIOC

/* ---- Motor B (1) Config ---- */
#define MOTORB_PWM_PIN 	        GPIO_PIN_15
#define MOTORB_PWM_PORT         GPIOB
#define MOTORB_PWM_TIM_INSTANCE TIM15
#define MOTORB_PWM_TIM_AF       GPIO_AF1_TIM15
#define MOTORB_PWM_TIM_CHANNEL  TIM_CHANNEL_2

#define MOTORB_INA_PIN 	        GPIO_PIN_8
#define MOTORB_INA_PORT         GPIOA
#define MOTORB_INB_PIN 	        GPIO_PIN_9
#define MOTORB_INB_PORT         GPIOC
#define MOTORB_ENA_PIN 	        GPIO_PIN_14
#define MOTORB_ENA_PORT         GPIOB
#define MOTORB_ENB_PIN 	        GPIO_PIN_13
#define MOTORB_ENB_PORT	        GPIOB

/** Period of motor control loop */
#define MOTOR_CONTROL_PERIOD    100

extern int32_t desired_positions[2];
extern arm_pid_instance_f32 pid[2];

extern bool flag_motor_running;


int32_t last_run_times[2];


#endif /* MAIN_H_ */
