#ifndef CLOCKS_H_
#define CLOCKS_H_

#include "stm32f3xx.h"

/** @brief Initialize and enable required clocks
 * 
 * Sets SYSCLK to 64MHz, from 8MHz crystal.
 */
void clocks_init(void);

#endif