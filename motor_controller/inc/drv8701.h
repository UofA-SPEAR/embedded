#ifndef DRV8701_H_
#define DRV8701_H_

#include "hal.h"

void drv8701_init(void);
void drv8701_set(int16_t velocity);
void drv8701_stop(void);
void drv8701_set_current(dacsample_t current):

#endif // DRV8701_H_