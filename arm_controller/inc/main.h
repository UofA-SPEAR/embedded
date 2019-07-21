/*
 * main.h
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


extern int32_t desired_positions[2];
extern arm_pid_instance_f32 pid[2];


int32_t last_run_times[2];


#endif /* MAIN_H_ */
