/*
 * coms.c
 *
 *  Created on: Feb 11, 2019
 *      Author: isthatme
 */

#include <stdbool.h>

#include "stm32f3xx.h"

#include "canard.h"
#include "canard_stm32.h"
#include "coms.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"

static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];
static uint8_t dynamic_array_buf[LIBCANARD_STM32_DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

static void com_peripherals_init();

bool should_accept(const CanardInstance* ins,
		uint64_t * out_data_type_signature,
		uint16_t data_type_id,
		CanardTransferType transfer_type,
		uint8_t source_node_id) {

	// This is all we want to handle
	if (data_type_id == UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID) {
		*out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
		return true;
	}

	return false;
}

void on_reception(CanardInstance* ins,
		CanardRxTransfer* transfer) {

	// We can assume we are only handling arraycommands
	uavcan_equipment_actuator_ArrayCommand msg;
	uavcan_equipment_actuator_ArrayCommand_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);

	for (int i = 0; i < msg.commands.len; i++) {
		uavcan_equipment_actuator_Command *cmd = &(msg.commands.data[i]);
		switch (cmd->actuator_id) {
		case (0):
			// handle motor 0
			break;
		case (1):
			// handle motor 1
			break;
		case (2):
			// handle motor 2
			break;
		case (3):
			// handle motor 3
			break;
		default:
			// Not any of these motors
			break;
		}
	}

}

void libcanard_init() {
	com_peripherals_init();

	canardInit(&m_canard_ins,
			&libcanard_memory_pool,
			LIBCANARD_MEM_POOL_SIZE,
			on_reception,
			should_accept,
			NULL);

	CanardSTM32CANTimings canbus_timings;
	canardSTM32ComputeCANTimings(8000000, 250000, &canbus_timings);


	canardSTM32Init(&canbus_timings, CanardSTM32IfaceModeNormal);
}

// Timestamp should come from the input of this
int8_t rx_once() {
	CanardCANFrame in_frame;

	int16_t rc = canardSTM32Receive(&in_frame);

	switch (rc) {
	case 1:
		canardHandleRxFrame(&m_canard_ins, &in_frame, 0); // There should be timestamp but who cares
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
}

static void com_peripherals_init() {
	// Enable clocks
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Initialize GPIO (CAN_RX and CAN_TX)
	GPIO_InitTypeDef gpio;
	gpio.Pin 		= GPIO_PIN_11 | GPIO_PIN_12;
	gpio.Mode 		= GPIO_MODE_AF_PP;
	gpio.Pull 		= GPIO_NOPULL;
	gpio.Speed 		= GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate 	= GPIO_AF9_CAN;
	HAL_GPIO_Init(GPIOA, &gpio);
}

// Function literally only needed for initialization bit of CAN
// DON'T USE
void usleep(useconds_t us) {
	HAL_Delay(1);
}
