#ifndef EMS22_H_
#define EMS22_H_

#include <string.h>
#include "stm32f3xx.h"


/** @brief Function to initialise ems22 setup
 * 
 *
 */
void ems22_init();

/** @brief Function to read position of ems22 */
int16_t ems22_read_position(uint8_t motor);

#endif