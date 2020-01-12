/*
 * can_fifo.h
 *
 *  Created on: Feb 9, 2019
 *      Author: isthatme
 */

#ifndef CAN_FIFO_H_
#define CAN_FIFO_H_

#include "canard.h"

#define FIFO_BUFFER_SIZE 20

typedef enum {
	FIFO_OK = 0,
	FIFO_FULL,
	FIFO_EMPTY,
	FIFO_ERROR,
} fifo_ret_t;

/** @brief initializes FIFO variables
 *
 */
fifo_ret_t fifo_init();

/** @brief Pushes CAN frame onto FIFO
 *
 * @retval FIFO_OK Everything is cool
 * @retval FIFO_FULL No more space in FIFO
 * @retval FIFO_ERROR catch-all for errors
 *
 */
fifo_ret_t fifo_push(CanardCANFrame* in_frame);

/** @brief Pops CAN frame off of FIFO
 *
 * @retval FIFO_OK Everything is cool
 * @retval FIFO_EMPTY No frames in buffer
 * @retval FIFO_ERROR catch-all for errors
 */
fifo_ret_t fifo_pop(CanardCANFrame* out_frame);

/** @brief Reads frame from FIFO without popping
 *
 * @retval FIFO_OK Everything is cool
 * @retval FIFO_EMPTY No frames in buffer
 * @retval FIFO_ERROR catch-all for errors
 */
fifo_ret_t fifo_read(CanardCANFrame* read_frame);

#endif /* CAN_FIFO_H_ */
