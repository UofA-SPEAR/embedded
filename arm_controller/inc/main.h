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

typedef enum {
	ENCODER_POTENTIOMETER, // Analog potentiometer (or any analog value)
	ENCODER_QUADRATURE, // Any quadrature encoder
	ENCODER_ABSOLUTE_DIGITAL, // specifically the digital encoder we are using
} encoder_type_t;


extern float motorA_desired_position;
extern float motorB_desired_position;

// Insanely large number, so no motor checks should happen.
extern int32_t last_runA;
extern int32_t last_runB;


#endif /* MAIN_H_ */
