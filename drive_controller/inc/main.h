/*
 * main.h
 *
 *  Created on: Feb 12, 2019
 *      Author: isthatme
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "sabertooth.h"

// Time (in ms) from last received message to stop motors
#define MOTOR_TIMEOUT_MS 1000

sabertooth_t saberA, saberB;

uint32_t timeout;

#endif /* MAIN_H_ */
