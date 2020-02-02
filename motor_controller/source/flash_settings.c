/*
 * flash_settings.c
 *
 *  Created on: Jan 31, 2019
 *      Author: David Lenfesty
 */

#include "flash_settings.h"
#include "settings.h"
#include "main.h"
#include "hal.h"

#define FLASH_WAIT_BUSY() \
	asm("nop"); \
	while(FLASH->SR & FLASH_SR_BSY)

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
	if (saved_settings[get_id_by_name("spear.motor.firstboot")].value.integer == -1) {
		current_settings[get_id_by_name("spear.motor.firstboot")].value.integer = 1;
        current_settings[get_id_by_name("spear.motor.enabled")].value.boolean = 0;
        current_settings[get_id_by_name("spear.motor.actuator_id")].value.integer = 42; 
        current_settings[get_id_by_name("spear.motor.reversed")].value.boolean = 0;
        current_settings[get_id_by_name("spear.motor.continuous")].value.boolean = 0;

        current_settings[get_id_by_name("spear.motor.pid.Kp")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.pid.Ki")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.pid.Kd")].value.real = 0;

        current_settings[get_id_by_name("spear.motor.encoder.type")].value.integer = ENCODER_POTENTIOMETER;
        current_settings[get_id_by_name("spear.motor.encoder.min")].value.integer = 0;
        current_settings[get_id_by_name("spear.motor.encoder.max")].value.integer = 0;
        current_settings[get_id_by_name("spear.motor.encoder.to_radians")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.encoder.endstop_min")].value.integer = ENDSTOP_DISABLED;
        current_settings[get_id_by_name("spear.motor.encoder.endstop_max")].value.integer = ENDSTOP_DISABLED;

        current_settings[get_id_by_name("spear.motor.linear.support_length")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.linear.arm_length")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.linear.length_min")].value.real = 0;
        current_settings[get_id_by_name("spear.motor.linear.length_max")].value.real = 0;
		program_settings(); // Write settings to flash
	}

}

// Load settings from flash
void load_settings(void) {
	firstboot_check();

	for (int i = 0; i < NUM_SETTINGS; i++) {
		current_settings[i].value = saved_settings[i].value;
	}
}

void program_settings(void)
{
	// Pointer assignments to make things not look like garbage
	uint16_t* p_saved_settings = (uint16_t*) &saved_settings;
	uint16_t* p_current_settings = (uint16_t*) &current_settings;

	// Unlock flash
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;

	// TODO actual error handling if flash remains locked
	if (FLASH->CR & FLASH_CR_LOCK)
		while (1);

	// Erase one page of flash
	while (FLASH->SR & FLASH_SR_BSY); // Wait for flash to not be busy
	FLASH->CR = FLASH_CR_PER; // Select page erase
	FLASH->AR = (uint32_t) &saved_settings;
	FLASH->CR |= FLASH_CR_STRT;
	FLASH_WAIT_BUSY();
	if (FLASH->SR & FLASH_SR_EOP) { // flash operation complete
		FLASH->SR |= FLASH_SR_EOP;
	} else {
		// TODO actual error handling
		while(1);
	}

	// Loop through half words (16 bits) of settings
	for (uint32_t i = 0; i < ROUND_UP(sizeof(saved_settings), 2); i++) {
		uint16_t *write_addr = p_saved_settings + i;
		uint16_t *read_addr = p_current_settings + i;
		FLASH->CR = FLASH_CR_PG; // select program
		*write_addr = *read_addr; // write data
		FLASH_WAIT_BUSY();
		if (FLASH->SR & FLASH_SR_EOP) { // flash operation complete
			FLASH->SR |= FLASH_SR_EOP; // clear flag
		} else {
			// TODO actuall error handling
			while(1);
		}
	}

	// Re-lock flash
	FLASH->CR |= FLASH_CR_LOCK;
}
