/*
 * coms.h
 *
 *  Created on: Feb 11, 2019
 *      Author: isthatme
 */

#ifndef COMS_H_
#define COMS_H_

#include "canard.h"

#include "uavcan/protocol/NodeStatus.h"

/* ------------ Error Definitions --------------- */

#define LIBCANARD_SUCCESS					1
#define LIBCANARD_NO_QUEUE					0
#define LIBCANARD_ERR						-1
#define LIBCANARD_ERR_NO_MEM				-2
#define LIBCANARD_ERR_INVALID_SETTINGS		-3
#define LIBCANARD_ERR_TX_QUEUE_FULL			-4

/* ------------ Memory Pool Definitions ---------- */
#define LIBCANARD_MEM_POOL_SIZE 1024
#define LIBCANARD_STM32_DYNAMIC_ARRAY_BUF_SIZE 256

extern CanardInstance m_canard_ins;
extern uint8_t inout_transfer_id;


void libcanard_init();

int8_t rx_once();

int8_t tx_once();

void coms_send_NodeStatus(uint8_t health, uint8_t mode, uint16_t vs_status);

int16_t setup_hardware_can_filters(void);

int8_t handle_frame(void);


void coms_odom_broadcast(uint8_t wheel, float delta);

#endif /* COMS_H_ */
