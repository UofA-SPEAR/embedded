/*
 * settings.h
 *
 *  Created on: Feb 3, 2019
 *      Author: David lenfesty
 *
 *  Handles settings requests and the such coming in from CAN.
 *
 *  Assumes you know what you're doing when you set the values.
 *  If you don't, very bad things could happen and you should realise that.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "canard.h"

#define NUM_PARAMETERS 34

void handle_getSet(CanardInstance* ins, CanardRxTransfer* transfer);

#endif /* SETTINGS_H_ */
