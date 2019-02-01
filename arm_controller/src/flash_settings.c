/*
 * flash_settings.c
 *
 *  Created on: Jan 31, 2019
 *      Author: isthatme
 */

#include <stm32f3xx.h>
#include "flash_settings.h"
#include "main.h"

// Settings saved to flash. Points to a specific flash location
__attribute__((section("._user_settings"))) flash_settings_t saved_settings;

// Settings in use.
flash_settings_t current_settings;

void do_thing(void) {
	saved_settings.motor1.actuator_id++;
}
