/*
 * libcanard_wrapper.h
 *
 *  Created on: Oct 28, 2018
 *      Author: David Lenfesty
 */

#ifndef LIBCANARD_WRAPPER_H_
#define LIBCANARD_WRAPPER_H_

#include "canard.h"

/* ------------ Error Definitions --------------- */

#define LIBCANARD_SUCCESS					1
#define LIBCANARD_NO_QUEUE					0
#define LIBCANARD_ERR						-1
#define LIBCANARD_ERR_NO_MEM				-2
#define LIBCANARD_ERR_INVALID_SETTINGS		-3
#define LIBCANARD_ERR_TX_QUEUE_FULL			-4


/* ------------ Filtering Mask Definitions ------ */

#define CAN_MASK_UAVCAN_PRIORITY		(0b11111 << 24)
#define CAN_MASK_UAVCAN_MSG_TYPE		(0b1111111111111111 << 8)
#define CAN_MASK_UAVCAN_SRV_NOT_MSG		(0b1 << 7)
#define CAN_MASK_UAVCAN_SRC_NODE_ID		(0b1111111 << 0)

/* ------------ Value Definitions ---------------- */

// Memory pool size. Minimum is 1K.
#ifndef LIBCANARD_MEM_POOL_SIZE
#define LIBCANARD_MEM_POOL_SIZE 1024 // Default to 1K
#endif



CanardInstance m_canard_instance;


/** @brief Function to initialize libcanard stuff.
 *
 * 	@param instance Pointer to uninitialized CanardInstance.
 *
 *  @param on_reception Callback function called when a transfer
 *  				is received.
 *
 * 	@param should_accept Callback function to determine if a frame should
 * 					be accepted.
 *
 * 	@param user_reference User pointer held within the CanardInstance.
 *
 *	@param clock_rate Speed in Hz of peripheral clock used for CAN.
 *
 * 	@param bitrate	CAN bitrate to achieve for with CAN timings.
 *
 *
 * 	@retval 0 Failure
 * 	@retval 1 Success
 */
int16_t libcanard_init(	CanardOnTransferReception on_reception,
						CanardShouldAcceptTransfer should_accept,
						void* user_reference,
						const uint32_t clock_rate,
						const uint32_t bitrate);



/** @brief Moves one frame from the tx queue to the hardware tx queue.
 *
 * @retval 1 Transmitted one frame.
 * @retval 0 No frames to transmit.
 */
int8_t tx_once(void);



/** @brief Receives one frame from the hardware rx queue
 * 		and processes it through libcanard.
 *
 * 	@note Will possibly call should_accept callback. Only call
 * 		this function when you have time.
 *
 * 	@return Not sure what this should return.
 */
int8_t rx_once(void);


#endif /* LIBCANARD_WRAPPER_H_ */
