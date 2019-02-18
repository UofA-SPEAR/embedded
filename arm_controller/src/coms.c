#include <stdbool.h>

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include "canard.h"
#include "canard_stm32.h"
#include "coms.h"
#include "main.h"
#include "flash_settings.h"
#include "settings.h"
#include "can_fifo.h"

#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"
#include "uavcan/protocol/param/GetSet.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/RestartNode.h"
#include "uavcan/protocol/GetNodeInfo.h"

// Small enough to not be too bad, large enough to be useful
#define DYNAMIC_ARRAY_BUF_SIZE 1000

CanardInstance m_canard_instance;
static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

// Dynamic array buffer for decoding messages
static uint8_t dynamic_array_buf[DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

uint8_t out_buf[100];

uint8_t inout_transfer_id;

TIM_HandleTypeDef htim7;

uint64_t can_timestamp_usec;

static void timestamp_tim_init(void);
static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer);
static void return_node_info(CanardInstance* ins, CanardRxTransfer* transfer);


void updateComs(void) {
	tx_once();
	rx_once();
}

void comInit(void) {
	timestamp_tim_init();
	fifo_init();
	libcanard_init(on_reception, should_accept, NULL, 32000000, 250000);
	setup_hardware_can_filters();
    // Configure interrupts
    // We only need to worry about the RX FIFO 0, because that's how the CAN interface is by default
    NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 1);
    NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
    CAN->IER |= 1 << CAN_IER_FMPIE0_Pos; // Enable CAN interrupt
}

// Should this be moved somewhere else?
static uint32_t radial_position_get(uint8_t motor, float in_angle) {
	uint32_t position;


	// radians / (radians/integer) = integers
	position = in_angle / run_settings.motor[motor].encoder.to_radians;

	// Needs to start at encoder_min
	position += run_settings.motor[motor].encoder.min;

	if (position > run_settings.motor[motor].encoder.max) {
		// TODO set nodestatus to error here

		// Probably the most sane thing to do in this case
		position = run_settings.motor[motor].encoder.max;
	}

	return position;
}

// Should this be moved somewhere else?
static uint32_t linear_position_get(uint8_t motor, float in_angle) {
	float desired_length;
	uint32_t position;

	// Hoping these get optimized out
	float* p_support_length = &(run_settings.motor[motor].linear.support_length);
	float* p_arm_length = &(run_settings.motor[motor].linear.arm_length);

	// Comes from cosine law
	// c^2 = a^2 + b^2 - 2ab*cos(C)
	desired_length = sqrt(
				pow(*p_support_length, 2) +
				pow(*p_support_length, 2) -
				(2 * (*p_support_length) * (*p_arm_length) * cos(in_angle))
			);

	// TODO set nodestatus
	if (desired_length < run_settings.motor[motor].linear.length_min) {
		desired_length = run_settings.motor[motor].linear.length_min;
	} else if (desired_length > run_settings.motor[motor].linear.length_max) {
		desired_length = run_settings.motor[motor].linear.length_max;
	}

	// These are checked to be positive in check_settings()
	uint32_t encoder_range = run_settings.motor[motor].encoder.max -
			run_settings.motor[motor].encoder.max;
	float linear_range = run_settings.motor[motor].linear.length_max -
			run_settings.motor[motor].linear.length_min;



	// set position properly
	position =
			// fit length into encoder range
			(desired_length - run_settings.motor[motor].linear.length_min) *
			// Convert from length range into the encoder range
			(encoder_range / linear_range) +
			// Add the minimum encoder value
			run_settings.motor[motor].encoder.min;


	return position;
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

		uint32_t desired_position;

		for (uint8_t i = 0; i < 2; i++) {
			if (cmd->actuator_id == run_settings.motor[i].actuator_id) {
				if (run_settings.motor[i].encoder.to_radians != 0) {
					desired_position = radial_position_get(i, cmd->command_value);
				} else if (run_settings.motor[i].linear.support_length != 0) {
					desired_position = linear_position_get(i, cmd->command_value);
				} else {
					// do nothing I guess, the motors aren't enabled
				}
				break; // We can exit the loop
			}
		}

		if (cmd->actuator_id == run_settings.motor[0].actuator_id) {
			motorA_desired_position = desired_position;
			// "Start" motor A if unstarted
			if (last_runA == INT16_MAX) {
				last_runA = HAL_GetTick();
			}
		} else if (cmd->actuator_id == run_settings.motor[0].actuator_id) {
			motorB_desired_position = desired_position;
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
			*out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
			return true;
		case (UAVCAN_PROTOCOL_NODESTATUS_ID):
			return false;
		default:
			return false;
		}
	}

	if (transfer_type == CanardTransferTypeRequest) {
		switch (data_type_id) {
		case (UAVCAN_PROTOCOL_PARAM_GETSET_ID):
			*out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
			return true;
		case (UAVCAN_PROTOCOL_RESTARTNODE_ID):
			return true;
		case (UAVCAN_PROTOCOL_GETNODEINFO_ID):
			return true;
		default:
			return false;
		}
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
		case (UAVCAN_PROTOCOL_RESTARTNODE_ID):
			restart_node(ins, transfer);
			break;
		case (UAVCAN_PROTOCOL_GETNODEINFO_ID):
			return_node_info(ins, transfer);
			break;
		default:
			break;
		}
	}

}

static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer) {
	uavcan_protocol_RestartNodeRequest msg;

	uavcan_protocol_RestartNodeRequest_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);


	if (msg.magic_number == UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAGIC_NUMBER) {
		NVIC_SystemReset();
	}
}

static void return_node_info(CanardInstance* ins, CanardRxTransfer* transfer) {
	uavcan_protocol_GetNodeInfoResponse out_msg;

	out_msg.name.len = strlen("Arm Controller");
	out_msg.name.data = (uint8_t*) "Arm Controller";
	out_msg.software_version.major = 0;
	out_msg.software_version.minor = 1;
	out_msg.hardware_version.major = 0;
	out_msg.hardware_version.minor = 1;
	out_msg.hardware_version.certificate_of_authenticity.len = 0;

	// TODO hook this into main status
	out_msg.status.health 	= UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	out_msg.status.mode 	= UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
	out_msg.status.sub_mode = 0;
	out_msg.status.vendor_specific_status_code = 0;
	out_msg.status.uptime_sec = HAL_GetTick() / 1000;

	uint8_t len = uavcan_protocol_GetNodeInfoResponse_encode(&out_msg, out_buf);

	canardRequestOrRespond(ins,
			transfer->source_node_id,
			UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
			UAVCAN_PROTOCOL_GETNODEINFO_ID,
			&inout_transfer_id,
			30, // again with the priorities
			CanardResponse,
			out_buf,
			len);
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
		fifo_push(&in_frame);
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
}

int8_t handle_frame() {
	CanardCANFrame frame;

	if (fifo_pop(&frame) == FIFO_OK) {
		canardHandleRxFrame(&m_canard_instance, &frame, can_timestamp_usec + TIM7->CNT);
		return LIBCANARD_SUCCESS;
	} else {
		// something I guess
	}

	return LIBCANARD_ERR;
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

void USB_LP_CAN_RX0_IRQHandler(void) {
	// Not sure if this will turn out to be a good idea.
	rx_once();
}

static void timestamp_tim_init(void) {
	__HAL_RCC_TIM7_CLK_ENABLE();

	// Initialize to run at 1MHz
	// Reset every 1ms
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 32;
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

void TIM7_IRQHandler() {
	__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
	can_timestamp_usec += TIM2->CNT;
	// Updates once a millisecond
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
		msg.vendor_specific_status_code = 0;
		msg.uptime_sec = HAL_GetTick() / 1000;

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
