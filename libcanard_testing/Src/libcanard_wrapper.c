/*
 * libcanard_wrapper.c
 *
 *  Created on: Oct 28, 2018
 *      Author: David Lenfesty
 */


#include "canard.h"
#include "canard_stm32.h"
#include "libcanard_wrapper.h"

static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

int16_t libcanard_init(	CanardOnTransferReception on_reception,
						CanardShouldAcceptTransfer should_accept,
						void* user_reference,
						const uint32_t clock_rate,
						const uint32_t bitrate) {

	// Initializes the libcanard instance
	canardInit(	&m_canard_instance,
				&libcanard_memory_pool,
				LIBCANARD_MEM_POOL_SIZE,
				on_reception,
				should_accept,
				user_reference);


	// Computes optimal timings based on peripheral clock
	// and the bitrate you want.
	CanardSTM32CANTimings canbus_timings;
	if (canardSTM32ComputeCANTimings(	clock_rate,
										bitrate,
										&canbus_timings) != 0 ) {
		// Returns if the function can't compute with the given settings.
		return LIBCANARD_ERR_INVALID_SETTINGS;
	}

	// Initialize using calculated timings and in the normal mode.
	int16_t rc = canardSTM32Init(&canbus_timings,
								CanardSTM32IfaceModeNormal);


	// Temporary thing
	canardSetLocalNodeID(&m_canard_instance, 42);

	return rc;
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

	// Honestly this function is kind of poorly written right now.
	// I just want to get something out the door.
	return retval;
}

// Timestamp should come from the input of this
int8_t rx_once() {
	CanardCANFrame in_frame;

	int16_t rc = canardSTM32Receive(&in_frame);

	switch (rc) {
	case 1:
		canardHandleRxFrame(&m_canard_instance,
							&in_frame,
							100000); // timestamp needs to be dynamic. Do later
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
}
