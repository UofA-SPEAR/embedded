/*
 * flash_settings.c
 *
 *  Created on: Jan 31, 2019
 *      Author: David Lenfesty
 */

#include <stm32f3xx.h>
#include "flash_settings.h"
#include "main.h"

// Settings saved to flash. Points to a specific flash location
__attribute__((section("._user_settings"))) struct setting saved_settings[NUM_SETTINGS];

// Settings in use.
struct setting current_settings[NUM_SETTINGS];
// Run settings
struct setting run_settings[NUM_SETTINGS];


// If this is the first boot, settings in the flash will be garbage.
// PID settings should theoretically reflect this
// NOT foolproof by any means but should help avoid a few major issues
static void firstboot_check(void) {
	// Kp should be between -1 and 1, if not it was misconfigured.
	// Start with sane settings.
	if (saved_settings[get_id_by_name("spear.motor.firstboot")].boolean != 1) {
		current_settings[get_id_by_name("spear.motor.firstboot")].boolean = 1;
        current_settings[get_id_by_name("spear.motor.enabled")].boolean = 0;
        current_settings[get_id_by_name("spear.motor.actuator_id")].integer = 42; 
        current_settings[get_id_by_name("spear.motor.reversed")].boolean = 0;
        current_settings[get_id_by_name("spear.motor.continuous")].boolean = 0;

        current_settings[get_id_by_name("spear.motor.pid.Kp")].real = 0;
        current_settings[get_id_by_name("spear.motor.pid.Ki")].real = 0;
        current_settings[get_id_by_name("spear.motor.pid.Kd")].real = 0;

        current_settings[get_id_by_name("spear.motor.encoder.type")].integer = ENCODER_POTENTIOMETER;
        current_settings[get_id_by_name("spear.motor.encoder.min")].integer = 0;
        current_settings[get_id_by_name("spear.motor.encoder.max")].integer = 0;
        current_settings[get_id_by_name("spear.motor.encoder.to_radians")].real = 0;
        current_settings[get_id_by_name("spear.motor.encoder.endstop_min")].integer = ENDSTOP_DISABLED;
        current_settings[get_id_by_name("spear.motor.encoder.endstop_max")].integer = ENDSTOP_DISABLED;

        current_settings[get_id_by_name("spear.motor.linear.support_length")].real = 0;
        current_settings[get_id_by_name("spear.motor.linear.arm_length")].real = 0;
        current_settings[get_id_by_name("spear.motor.linear.length_min")].real = 0;
        current_settings[get_id_by_name("spear.motor.linear.length_max")].real = 0;
		program_settings(); // Write settings to flash
	}

}

// Load settings from flash
void load_settings(void) {
	firstboot_check();

	current_settings = saved_settings;
}

HAL_StatusTypeDef program_settings(void) {
	HAL_StatusTypeDef rc;

	// Need to unlock the flash before we start
	rc = HAL_FLASH_Unlock();

	if (rc != HAL_OK) { while(1); }

	// Assuming we only have one page of data to erase
	FLASH_EraseInitTypeDef Erase_Init;
	uint32_t page_error;
	Erase_Init.NbPages = 1;
	Erase_Init.PageAddress = (uint32_t) &saved_settings;
	Erase_Init.TypeErase = FLASH_TYPEERASE_PAGES;
	HAL_FLASHEx_Erase(&Erase_Init, &page_error);

	// More to this, but page_error will be 0xFFFFFFFF if it works.
	if (page_error != 0xFFFFFFFF) { while(1); }

	// Pointer assignments to make things look pretty
	uint32_t* p_saved_settings = (uint32_t*) &saved_settings;
	uint32_t* p_current_settings = (uint32_t*) &current_settings;


	// Loop through words (32 bits) of settings
	for (uint32_t i = 0; i < ROUND_UP(sizeof(saved_settings), 4); i++) {
		rc = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) (p_saved_settings + i),
				(uint32_t) *(p_current_settings + i));
		if (rc != HAL_OK) { while(1); };
	}

	// At this point, all the flash we care about should be written to,
	// so lock flash down just in case.
	rc = HAL_FLASH_Lock();
	return rc;
}
