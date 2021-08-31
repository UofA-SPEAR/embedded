/*
 * flash_settings.h
 *
 *  Created on: Jan 31, 2019
 *      Author: isthatme
 */

#ifndef FLASH_SETTINGS_H_
#define FLASH_SETTINGS_H_

#include "encoders.h"
#include "main.h"

typedef enum {
  ENDSTOP_DISABLED,
  ENDSTOP_ENABLED,
  ENDSTOP_ENABLED_INVERTED,
} endstop_t;

/** @brief Loads in user settings from flash.
 *
 * @note also checks if this is the firstboot, and sets decent default values
 */
void load_settings(void);

/** @brief writes current user settings into flash memory.
 *
 */
void program_settings(void);

#endif /* FLASH_SETTINGS_H_ */
