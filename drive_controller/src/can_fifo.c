/*
 * can_fifo.c
 *
 *  Created on: Feb 9, 2019
 *      Author: isthatme
 */

#include "can_fifo.h"

static struct {
	CanardCANFrame buffer[FIFO_BUFFER_SIZE];
	uint32_t wr_ptr;
	uint32_t rd_ptr;
	uint32_t num;
} frame_buf;


fifo_ret_t fifo_init() {
	frame_buf.wr_ptr = 0;
	frame_buf.rd_ptr = 0;
	frame_buf.num = 0;

	return FIFO_OK;
}


// I'm just dropping frames that don't fit in the buffer.
fifo_ret_t fifo_push(CanardCANFrame* in_frame) {
	if (frame_buf.num >= FIFO_BUFFER_SIZE) {
		return FIFO_FULL;
	}

	frame_buf.buffer[frame_buf.wr_ptr] = *in_frame;
	frame_buf.wr_ptr++;
	frame_buf.num++;

	// Deal with wraparounds (this is a circular buffer)
	if (frame_buf.wr_ptr >= FIFO_BUFFER_SIZE) {
		frame_buf.wr_ptr = 0;
	}
	return FIFO_OK;
}


fifo_ret_t fifo_pop(CanardCANFrame* out_frame) {
	// Should return immediately if FIFO is empty
	if (frame_buf.num == 0) {
		return FIFO_EMPTY;
	}


	// Pop frame out of buffer.
	*out_frame = frame_buf.buffer[frame_buf.rd_ptr];
	frame_buf.rd_ptr++;
	frame_buf.num--;

	// Deal with wraparounds (this is a circular buffer)
	if (frame_buf.rd_ptr >= FIFO_BUFFER_SIZE) {
		frame_buf.rd_ptr = 0;
	}

	return FIFO_OK;
}

fifo_ret_t fifo_read(CanardCANFrame* read_frame) {
	if (frame_buf.num == 0) {
		return FIFO_EMPTY;
	}

	*read_frame = frame_buf.buffer[frame_buf.rd_ptr];

	return FIFO_OK;
}
