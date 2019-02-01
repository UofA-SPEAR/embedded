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
__attribute__((section("._user_settings"))) flash_settings_t saved_settings;

// Settings in use.
flash_settings_t current_settings;


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
