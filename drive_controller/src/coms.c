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
#include "main.h"
#include "coms.h"
#include "sabertooth.h"
#include "can_fifo.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"

static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];
static uint8_t dynamic_array_buf[LIBCANARD_STM32_DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;
static uint8_t out_msg_buf[100];

static void com_peripherals_init();

TIM_HandleTypeDef htim7;
uint64_t can_timestamp_usec = 0;

CanardInstance m_canard_ins;

uint8_t inout_transfer_id = 0;

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

	// Reset timeout value so we don't just keep shutting the motors down.
	motor_timeout = HAL_GetTick();

	for (int i = 0; i < msg.commands.len; i++) {
		uavcan_equipment_actuator_Command *cmd = &(msg.commands.data[i]);
		int8_t speed = msg.commands.data[i].command_value;

		switch (cmd->actuator_id) {
		case (0):
			// handle motor 0
			motorB_speed = speed;
			break;
		case (1):
			// handle motor 1
			motorB_speed = speed;
			break;
		case (2):
			// handle motor 2
			motorA_speed = speed;
			break;
		case (3):
			// handle motor 3
			motorA_speed = speed;
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
	canardSTM32ComputeCANTimings(32000000, 250000, &canbus_timings);

	// TODO: Get this doing the right thing.
	canardSetLocalNodeID(&m_canard_ins, 20);


	canardSTM32Init(&canbus_timings, CanardSTM32IfaceModeNormal);

	setup_hardware_can_filters();

	NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 1);
	NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
	CAN->IER |= 1 << CAN_IER_FMPIE0_Pos;

	__HAL_RCC_TIM7_CLK_ENABLE();

	// Initialize to run at 1MHz
	// Reset every 1ms
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 63;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 1000;
	htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim7);

	// Make it independent
	TIM_MasterConfigTypeDef sMasterConfig;
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);


	// Enable interrupts
	NVIC_SetPriority(TIM7_IRQn, 0);
	NVIC_EnableIRQ(TIM7_IRQn);

	// Start interrupts
	HAL_TIM_Base_Start_IT(&htim7);
}

void USB_LP_CAN_RX0_IRQHandler(void) {
	rx_once();
}

void TIM7_IRQHandler(void) {
	__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
	can_timestamp_usec += TIM7->CNT;
}

// Timestamp should come from the input of this
int8_t rx_once() {
	CanardCANFrame in_frame;

	int16_t rc = canardSTM32Receive(&in_frame);

	switch (rc) {
	case 1:
		rc = fifo_push(&in_frame);
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
}

int8_t tx_once() {
	const CanardCANFrame* out_frame;

	out_frame = canardPeekTxQueue(&m_canard_ins);

	if (out_frame != NULL) {
		switch (canardSTM32Transmit(out_frame)) {
			case (1):
				canardPopTxQueue(&m_canard_ins);
				break;
			case (0):
				// buffer full
				break;
			default:
				// Other error
				break;
		}
	}

	return 0;
}

int8_t handle_frame() {
	CanardCANFrame frame;

	if (fifo_pop(&frame) == FIFO_OK) {
		canardHandleRxFrame(&m_canard_ins, &frame, can_timestamp_usec + TIM7->CNT);
		return LIBCANARD_SUCCESS;
	} else {
		// something I guess
	}

	return LIBCANARD_ERR;
}


void coms_send_NodeStatus(uint8_t health, uint8_t mode, uint16_t vs_status) {
	uavcan_protocol_NodeStatus msg;

	msg.uptime_sec = HAL_GetTick() / 1000;
	msg.health = health;
	msg.mode = mode;
	msg.sub_mode = 0;
	msg.vendor_specific_status_code = vs_status;

	uint8_t len = uavcan_protocol_NodeStatus_encode(&msg, out_msg_buf);

	canardBroadcast(&m_canard_ins,
		UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
		UAVCAN_PROTOCOL_NODESTATUS_ID,
		&inout_transfer_id,
		0,
		&msg,
		len);
}

static void com_peripherals_init() {
	// Enable clocks
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_CAN1_CLK_ENABLE();

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

int16_t setup_hardware_can_filters(void) {
	const CanardSTM32AcceptanceFilterConfiguration conf[1] = { { .id = 0,
			.mask = 0 } };

	return canardSTM32ConfigureAcceptanceFilters(
			(const CanardSTM32AcceptanceFilterConfiguration* const ) &conf, 1);
}
