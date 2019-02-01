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


#endif /* MAIN_H_ */
