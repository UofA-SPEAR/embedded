#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f3xx.h"
#include "pins.h"
#include "clocks.h"
#include "coms.h"
#include "timers.h"
#include "adc.h"

// Control flags
extern bool flag_take_measurement;
extern bool flag_publish_battery;

#endif