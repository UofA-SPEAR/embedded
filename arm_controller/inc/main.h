/*
 * main.h
 *
 *  Created on: Jan 25, 2019
 *      Author: isthatme
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f3xx.h"

#define ROUND_UP(dividend, divisor) ((dividend + (divisor - 1)) / divisor)


extern float motorA_desired_position;
extern float motorB_desired_position;

int32_t last_runA;
int32_t last_runB;


#endif /* MAIN_H_ */
