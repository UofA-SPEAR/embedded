/*
 * flash_settings.c
 *
 *  Created on: Jan 31, 2019
 *      Author: David Lenfesty
 */

#include "flash_settings.h"

#include <string.h>

#include "hal.h"
#include "main.h"
#include "settings.h"

#define FLASH_WAIT_BUSY() \
  __asm__("nop");             \
  while (FLASH->SR & FLASH_SR_BSY)

// Settings saved to flash. Points to a specific flash location
__attribute__((section(
    "._user_settings"))) struct setting_value_t saved_settings[NUM_SETTINGS];

// Settings changed by CAN configuration
struct setting_value_t pending_settings[NUM_SETTINGS];
// Settings in use
struct setting_value_t current_settings[NUM_SETTINGS];

// If this is the first boot, settings in the flash will be garbage.
// PID settings should theoretically reflect this
// NOT foolproof by any means but should help avoid a few major issues
static void firstboot_check(void) {
  // Kp should be between -1 and 1, if not it was misconfigured.
  // Start with sane settings.
  if (setting_by_name(saved_settings, "spear.motor.firstboot")->value.integer ==
      -1) {
    memset(pending_settings, 0, sizeof(pending_settings));
    for (int i = 0; i < NUM_SETTINGS; i++) {
      pending_settings[i].union_tag = setting_specs[i].union_tag;
    }

    setting_by_name(pending_settings, "spear.motor.firstboot")->value.integer =
        1;
    setting_by_name(pending_settings, "spear.motor.actuator_id")
        ->value.integer = 42;

    setting_by_name(pending_settings, "spear.motor.encoder.type")
        ->value.integer = ENCODER_POTENTIOMETER;
    setting_by_name(pending_settings, "spear.motor.encoder.endstop_min")
        ->value.integer = ENDSTOP_DISABLED;
    setting_by_name(pending_settings, "spear.motor.encoder.endstop_max")
        ->value.integer = ENDSTOP_DISABLED;

    program_settings();  // Write settings to flash
  }
}

// Load settings from flash
void load_settings(void) {
  firstboot_check();

  for (int i = 0; i < NUM_SETTINGS; i++) {
    pending_settings[i] = saved_settings[i];
  }
  for (int i = 0; i < NUM_SETTINGS; i++) {
    current_settings[i] = saved_settings[i];
  }
}

void program_settings(void) {
  // Pointer assignments to make things not look like garbage
  uint16_t* p_saved_settings = (uint16_t*)&saved_settings;
  uint16_t* p_pending_settings = (uint16_t*)&pending_settings;

  // Unlock flash
  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;

  // TODO actual error handling if flash remains locked
  if (FLASH->CR & FLASH_CR_LOCK)
    while (1)
      ;

  // Erase one page of flash
  while (FLASH->SR & FLASH_SR_BSY)
    ;                        // Wait for flash to not be busy
  FLASH->CR = FLASH_CR_PER;  // Select page erase
  FLASH->AR = (uint32_t)&saved_settings;
  FLASH->CR |= FLASH_CR_STRT;
  FLASH_WAIT_BUSY();
  if (FLASH->SR & FLASH_SR_EOP) {  // flash operation complete
    FLASH->SR |= FLASH_SR_EOP;
  } else {
    // TODO actual error handling
    while (1)
      ;
  }

  // Loop through half words (16 bits) of settings
  for (uint32_t i = 0; i < ROUND_UP(sizeof(saved_settings), 2); i++) {
    uint16_t* write_addr = p_saved_settings + i;
    uint16_t* read_addr = p_pending_settings + i;
    FLASH->CR = FLASH_CR_PG;   // select program
    *write_addr = *read_addr;  // write data
    FLASH_WAIT_BUSY();
    if (FLASH->SR & FLASH_SR_EOP) {  // flash operation complete
      FLASH->SR |= FLASH_SR_EOP;     // clear flag
    } else {
      // TODO actuall error handling
      while (1)
        ;
    }
  }

  // Re-lock flash
  FLASH->CR |= FLASH_CR_LOCK;
}
