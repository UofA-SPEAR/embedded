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

/* TODO:
 * - change to nicer message handling infrastructure
 */

// Small enough to not be too bad, large enough to be useful
#define DYNAMIC_ARRAY_BUF_SIZE 	1000
#define RX_FIFO_LEN				10

CanardInstance m_canard_instance;
static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

// Dynamic array buffer for decoding messages
static uint8_t dynamic_array_buf[DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

uint8_t out_buf[100];

uint8_t inout_transfer_id;

static objects_fifo_t rx_fifo;
static CanardCANFrame rx_fifo_buffer[RX_FIFO_LEN];
static msg_t rx_fifo_msg_buffer[RX_FIFO_LEN];

static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer);
static void return_node_info(CanardInstance* ins, CanardRxTransfer* transfer);


CH_IRQ_HANDLER(CAN_RX1_IRQn)
{
	CH_IRQ_PROLOGUE();
	chSysLockFromISR();

	CanardCANFrame frame;

	// TODO check error here
	canardSTM32Receive(&frame);

	chFifoSendObjectI(&rx_fifo, (void*) &frame);

	chSysUnlockFromISR();
	CH_IRQ_EPILOGUE();
}

void comInit(void)
{
	CanardSTM32CANTimings timings;
	CANConfig CAN;

	chFifoObjectInit(&rx_fifo, sizeof(CanardCANFrame), RX_FIFO_LEN,
		STM32_NATURAL_ALIGNMENT, rx_fifo_buffer, rx_fifo_msg_buffer);

	canardSTM32ComputeCANTimings(72000000 / 2, 250000, &timings);	

	CAN = {
		.mcr = 0x00010002,
	}
	CAN.btr = (timings.bit_rate_prescaler << CAN_BTR_BRP_Pos) & CAN_BTR_BRP,
	CAN.btr |= (timings.bit_segment_1 << CAN_BTR_TS1_Pos) & CAN_BTR_TS1;
	CAN.btr |= (timings.bit_segment_2 << CAN_BTR_TS2_Pos) & CAN_BTR_TS2;
	CAN.btr |= (timings.max_resynchronization_jump_width << CAN_BTR_SJW_Pos)
				& CAN_BTR_SJW;

	canObjectInit(&CAND1);
	canStart(&CAND1, &CAN);
	libcanard_init(on_reception, should_accept, NULL, 32000000, 250000);
	// Sets to default filter
	canSTM32SetFilters(&CAND1, 0, 0, NULL);
}

/** 
 * @brief Takes control of thread to deal with coms.
 */
void coms_handle_forever()
{
	int8_t retval = LIBCANARD_SUCCESS;
	CanardCANFrame *frame;
	int16_t rc;

	while (1) {

		frame = chFifoTakeObjectI(&rx_fifo);

		if (frame != NULL)
			canardHandleRxFrame(&m_canard_instance, frame,
				TIME_I2MS(chVTGetSystemTimeX()));

		frame = canardPeekTxQueue(&m_canard_instance);

		if (frame != NULL) { // If there are any frames to transmit
			rc = canardSTM32Transmit(frame);

			if (rc == 1) { // If transmit is successful
				canardPopTxQueue(&m_canard_instance);
			} else if (rc == 0) { // If the TX queue is full
				retval = LIBCANARD_ERR_TX_QUEUE_FULL;
			} else {
				// TODO handle these errors properly
			}
		}
	}
}

// Should this be moved somewhere else?
static int32_t radial_position_get(uint8_t motor, float in_angle)
{
	int32_t position;


	if (run_settings.motor[motor].encoder.type == ENCODER_POTENTIOMETER) {
		// radians / (radians/integer) = integers
		position = in_angle / run_settings.motor[motor].encoder.to_radians;

		// Needs to start at encoder_min
		position += run_settings.motor[motor].encoder.min;

		if (position > run_settings.motor[motor].encoder.max) {
			node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING;

			// Probably the most sane thing to do in this case
			position = run_settings.motor[motor].encoder.max;
		}
	} else if (run_settings.motor[motor].encoder.type == ENCODER_QUADRATURE ||
			run_settings.motor[motor].encoder.type == ENCODER_ABSOLUTE_DIGITAL) {
		position = in_angle / run_settings.motor[motor].encoder.to_radians;
	} else {
		// mostly just to remove an error here.
		// bad error handling but whatever
		return 0;
	}

	return position;
}

// Should this be moved somewhere else?
static int32_t linear_position_get(uint8_t motor, float in_angle)
{
	float desired_length;
	int32_t position;

	// Hoping these get optimized out
	float* p_support_length = &(run_settings.motor[motor].linear.support_length);
	float* p_arm_length = &(run_settings.motor[motor].linear.arm_length);

	// Comes from cosine law
	// c^2 = a^2 + b^2 - 2ab*cos(C)
	desired_length = sqrt(
				pow(*p_support_length, 2) +
				pow(*p_arm_length, 2) -
				(2 * (*p_support_length) * (*p_arm_length) * cos(in_angle))
			);

	// TODO set nodestatus
	if (desired_length < run_settings.motor[motor].linear.length_min) {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING;
		desired_length = run_settings.motor[motor].linear.length_min;
	} else if (desired_length > run_settings.motor[motor].linear.length_max) {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING;
		desired_length = run_settings.motor[motor].linear.length_max;
	} else {
		node_health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
	}

	// These are checked to be positive in check_settings()
	uint32_t encoder_range = run_settings.motor[motor].encoder.max -
			run_settings.motor[motor].encoder.min;
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
static void handle_actuator_command(CanardInstance *ins,
	CanardRxTransfer *transfer)
{
	uavcan_equipment_actuator_ArrayCommand msg;

	// Pull message data
	uavcan_equipment_actuator_ArrayCommand_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);

	for (int i = 0; i < msg.commands.len; i++) {
		uavcan_equipment_actuator_Command* cmd = &msg.commands.data[i];

		int32_t desired_position = 0;

		for (uint8_t i = 0; i < 2; i++) {
			if (cmd->actuator_id == run_settings.motor[i].actuator_id) {
				if (run_settings.motor[i].encoder.to_radians != 0) {
					desired_position = radial_position_get(i, cmd->command_value);
				} else if (run_settings.motor[i].linear.support_length != 0) {
					desired_position = linear_position_get(i, cmd->command_value);
				} else {
					// do nothing I guess, the motors aren't enabled
					return;
				}
				break; // We can exit the loop
			}
		}

		if (cmd->actuator_id == run_settings.motor[0].actuator_id) {
			desired_positions[0] = desired_position;
			// "Start" motor A if unstarted
			if (last_run_times[0] == INT16_MAX) {
				last_run_times[0] = HAL_GetTick();
			}
		} else if (cmd->actuator_id == run_settings.motor[1].actuator_id) {
			desired_positions[1] = desired_position;
			// "Start" motor B if unstarted
			if (last_run_times[1] == INT16_MAX) {
				last_run_times[1] = HAL_GetTick();
			}
		}
	}

}

struct can_msg_handler can_request_handlers[] = {
	CAN_MSG_HANDLER(UAVCAN_PROTOCOL_PARAM_GETSET_ID,
		UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE, handle_getSet),
};

struct can_msg_handler can_broadcast_handlers[] = {
	CAN_MSG_HANDLER(UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_ID,
		UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_SIGNATURE, handle_actuator_command),
}

bool should_accept(const CanardInstance* ins,
					uint64_t* out_data_type_signature,
					uint16_t data_type_id,
					CanardTransferType transfer_type,
					uint8_t source_node_id)
	{

	if (transfer_type == CanardTransferTypeBroadcast) {
		for (int i = 0; i < NELEM(can_broadcast_handlers); i++) {
			if (data_type_id == can_broadcast_handlers[i].id) {
				*out_data_type_signature = can_broadcast_handlers[i].signature;
			}
		}
	}

	if (transfer_type == CanardTransferTypeRequest) {
		for (int i = 0; i < NELEM(can_request_handlers); i++) {
			if (data_type_id == can_request_handlers[i].id) {
				*out_data_type_signature = can_request_handlers[i].signature;
				return true;
			}
		}
	}

	return false;
}

void on_reception(CanardInstance* ins, CanardRxTransfer* transfer)
{
	if (transfer->transfer_type == CanardTransferTypeBroadcast) {
		for (int i = 0; i < NELEM(can_request_handlers; i++) {
			if (transfer->data_type_id == can_broadcast_handlers[i].id)
				can_broadcast_handlers[i].handler(ins,
					transfer);
		}
	}

	if (transfer->transfer_type == CanardTransferTypeRequest) {
		for (int i = 0; i < NELEM(can_request_handlers; i++) {
			if (transfer->data_type_id == can_request_handlers[i].id)
				can_request_handlers[i].handler(ins,
					transfer);
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
	out_msg.hardware_version.major = 1;
	out_msg.hardware_version.minor = 0;
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
		uint64_t timestamp = TIME_I2MS(chVTGetSystemTimeX());
		canardHandleRxFrame(&m_canard_instance, &frame, timestamp);
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
    // Enable CAN clock
	RCC->APB1ENR |= RCC_APB1ENR_CANEN;

	palSetPadMode(GPIOA, 11, PAL_STM32_ALTERNATE(9));
	palSetPadMode(GPIOA, 12, PAL_STM32_ALTERNATE(9));
}

void USB_LP_CAN_RX0_IRQHandler(void) {
	// Not sure if this will turn out to be a good idea.
	rx_once();
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

		msg.health = node_health;
		msg.mode   = node_mode;
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
