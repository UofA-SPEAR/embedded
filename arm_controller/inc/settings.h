/*
 * settings.h
 *
 *  Created on: Feb 3, 2019
 *      Author: David lenfesty
 *
 *  Handles settings requests and the such coming in from CAN.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "canard.h"

#define NUM_PARAMETERS 28

void handle_getSet(CanardInstance* ins, CanardRxTransfer* transfer);

#endif /* SETTINGS_H_ */
