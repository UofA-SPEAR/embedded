#include <stdbool.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "canard.h"
#include "canard_stm32.h"
#include "coms.h"
#include "main.h"
#include "flash_settings.h"
#include "settings.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"
#include "uavcan/protocol/param/GetSet.h"
#include "uavcan/protocol/NodeStatus.h"

// Small enough to not be too bad, large enough to be useful
#define DYNAMIC_ARRAY_BUF_SIZE 300

CanardInstance m_canard_instance;
static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

// Dynamic array buffer for decoding messages
static uint8_t dynamic_array_buf[DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

uint8_t out_buf[100];

uint8_t inout_transfer_id;


void updateComs(void) {
	tx_once();
	rx_once();
}

void comInit(void) {
	libcanard_init(on_reception, should_accept, NULL, 8000000, 250000);
	setup_hardware_can_filters();

}

/** @brief Handles ActuatorCommand messages
 *
 * Updates position values according to stuff
 */
static void handle_actuator_command(CanardRxTransfer* transfer) {
	uavcan_equipment_actuator_ArrayCommand msg;

	// Pull message data
	uavcan_equipment_actuator_ArrayCommand_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);

	for (int i = 0; i < msg.commands.len; i++) {
		uavcan_equipment_actuator_Command* cmd = &msg.commands.data[i];
		if (cmd->actuator_id == saved_settings.motor[0].actuator_id) {
			motorA_desired_position = cmd->command_value;
			// "Start" motor A if unstarted
			if (last_runA == INT16_MAX) {
				last_runA = HAL_GetTick();
			}
		} else if (cmd->actuator_id == saved_settings.motor[1].actuator_id) {
			// Set position
			motorB_desired_position = cmd->command_value;
			// "Start" motor B if unstarted
			if (last_runB == INT16_MAX) {
				last_runB = HAL_GetTick();
			}
		}
	}

}


bool should_accept(const CanardInstance* ins,
					uint64_t* out_data_type_signature,
					uint16_t data_type_id,
					CanardTransferType transfer_type,
					uint8_t source_node_id) {

	if (transfer_type == CanardTransferTypeBroadcast) {
		switch (data_type_id) {

		case (UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID):
			return true;
		case (UAVCAN_PROTOCOL_NODESTATUS_ID):
			return false;
		default:
			return false;
		}
	}

	if (transfer_type == CanardTransferTypeRequest) {
		return true;
	}

	return false;
}

void on_reception(CanardInstance* ins, CanardRxTransfer* transfer){
	if (transfer->transfer_type == CanardTransferTypeBroadcast) {
		switch (transfer->data_type_id) {
		case (UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID):
			handle_actuator_command(transfer);
			break;
		default:
			break;
		}
	}

	if (transfer->transfer_type == CanardTransferTypeRequest) {
		switch (transfer->data_type_id) {
		case(UAVCAN_PROTOCOL_PARAM_GETSET_ID):
			handle_getSet(ins, transfer);
			break;
		default:
			break;
		}
	}

}

int8_t tx_once(void) {
	int8_t retval = LIBCANARD_SUCCESS;
	int16_t rc;

	const CanardCANFrame* p_frame = canardPeekTxQueue(&m_canard_instance);

	if (p_frame != NULL) { // If there are any frames to transmit
		rc = canardSTM32Transmit(p_frame);

		if (rc == 1) { // If transmit is successful
			canardPopTxQueue(&m_canard_instance);
		} else if (rc == 0) { // If the TX queue is full
			retval = LIBCANARD_ERR_TX_QUEUE_FULL;
		} else {
			// TODO handle these errors properly
		}

	}
	return retval;
}

// Timestamp should come from the input of this
int8_t rx_once() {
	CanardCANFrame in_frame;

	int16_t rc = canardSTM32Receive(&in_frame);

	switch (rc) {
	case 1:
		// TODO setup timer for timestamp
		canardHandleRxFrame(&m_canard_instance, &in_frame, 100000);
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
}

/**
 * Initializes CAN clock and GPIO.
 *
 * PA11 -> CANRX
 * PA12 -> CANTX
 */
static void bxcan_init(void) {
    // Enable clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure PA11 & PA12 with CAN AF.
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

int16_t libcanard_init(CanardOnTransferReception on_reception,
		CanardShouldAcceptTransfer should_accept, void* user_reference,
		const uint32_t clock_rate, const uint32_t bitrate) {

	// Initializes the libcanard instance
	canardInit(&m_canard_instance, &libcanard_memory_pool,
	LIBCANARD_MEM_POOL_SIZE, on_reception, should_accept, user_reference);

	// Computes optimal timings based on peripheral clock
	// and the bitrate you want.
	CanardSTM32CANTimings canbus_timings;
	if (canardSTM32ComputeCANTimings(clock_rate, bitrate, &canbus_timings)
			!= 0) {
		// Returns if the function can't compute with the given settings.
		return LIBCANARD_ERR_INVALID_SETTINGS;
	}

	// Enable clocks and IO settings
	bxcan_init();
	// Initialize using calculated timings and in the normal mode.
	int16_t rc = canardSTM32Init(&canbus_timings, CanardSTM32IfaceModeNormal);

	return rc;
}
int16_t setup_hardware_can_filters(void) {
	const CanardSTM32AcceptanceFilterConfiguration conf[1] = { { .id = 0,
			.mask = 0 } };

	return canardSTM32ConfigureAcceptanceFilters(
			(const CanardSTM32AcceptanceFilterConfiguration* const ) &conf, 1);
}

void publish_nodeStatus(void) {
	static uint32_t last_time = 0;

	if (HAL_GetTick() - last_time > 1000) {
		last_time = HAL_GetTick();

		uavcan_protocol_NodeStatus msg;

		msg.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
		msg.mode   = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
		msg.sub_mode = 0;
		msg.vendor_specific_status_code = 14;
		msg.uptime_sec = 300;

		uint16_t len = uavcan_protocol_NodeStatus_encode(&msg, out_buf);

		canardBroadcast(&m_canard_instance,
				UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
				UAVCAN_PROTOCOL_NODESTATUS_ID,
				&inout_transfer_id,
				0,
				out_buf,
				len);
	}

}
