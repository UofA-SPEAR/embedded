#include <stdbool.h>

#include "canard.h"
//#include "canard_stm32.h"
#include "coms.h"
#include "main.h"
#include "flash_settings.h"
#include "settings.h"

#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"
#include "hal.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"
#include "uavcan/protocol/param/GetSet.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/RestartNode.h"
#include "uavcan/protocol/GetNodeInfo.h"


// Small enough to not be too bad, large enough to be useful
#define DYNAMIC_ARRAY_BUF_SIZE 	1000
#define RX_FIFO_LEN				10

CanardInstance m_canard_instance;
static uint8_t libcanard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

// Dynamic array buffer for decoding messages
static uint8_t dynamic_array_buf[DYNAMIC_ARRAY_BUF_SIZE];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

uint32_t node_health;
uint32_t node_mode;

uint8_t out_buf[100];

uint8_t inout_transfer_id;

static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer);
static void return_node_info(CanardInstance* ins, CanardRxTransfer* transfer);


static void chibiosCanardHandler(CanardCANFrame *rx_frame, const CanardCANFrame *tx_frame, CANRxFrame *rxmsg, CANTxFrame *txmsg) {
	if(txmsg != NULL) {
		txmsg->IDE = CAN_IDE_EXT;
		txmsg->EID = tx_frame->id;
		txmsg->RTR = CAN_RTR_DATA;
		txmsg->DLC = tx_frame->data_len;
		for(uint8_t i = 0; i < tx_frame->data_len; i++) {
			txmsg->data8[i] = tx_frame->data[i];
		}
	}
	else {
		rx_frame->id = rxmsg->EID;
		rx_frame->data_len = rxmsg->DLC;
		for(uint8_t i = 0; i < rx_frame->data_len; i++) {
			rx_frame->data[i] = rxmsg->data8[i];
		}
	}


}

void comInit(void)
{
//	CanardSTM32CANTimings timings;
//	CANConfig config;
//	// Enable CAN clock
//	RCC->APB1ENR |= RCC_APB1ENR_CANEN;

//	palSetPadMode(GPIOA, 11, PAL_STM32_ALTERNATE(9));
//	palSetPadMode(GPIOA, 12, PAL_STM32_ALTERNATE(9));
//
//	canardSTM32ComputeCANTimings(72000000 / 2, 250000, &timings);
//
//	config.mcr = 0x00010002;
//	config.btr = (timings.bit_rate_prescaler << CAN_BTR_BRP_Pos) & CAN_BTR_BRP_Msk,
//	config.btr |= (timings.bit_segment_1 << CAN_BTR_TS1_Pos) & CAN_BTR_TS1_Msk;
//	config.btr |= (timings.bit_segment_2 << CAN_BTR_TS2_Pos) & CAN_BTR_TS2_Msk;
//	config.btr |= (timings.max_resynchronization_jump_width << CAN_BTR_SJW_Pos)
//				& CAN_BTR_SJW_Msk;
	const CANConfig config = {
	/*.mcr = */ CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
	/*.btr = */ CAN_BTR_SJW(1) | CAN_BTR_TS2(4) |
	CAN_BTR_TS1(5) | CAN_BTR_BRP(5)
	};
	canStart(&CAND1, &config);

	canardInit(&m_canard_instance, &libcanard_memory_pool,
		LIBCANARD_MEM_POOL_SIZE, on_reception, should_accept,
		NULL);

	// Sets to default filter
	canSTM32SetFilters(&CAND1, 0, 0, NULL);
}

/** 
 * @brief Takes control of thread to deal with coms.
 */
void coms_handle_forever(void)
{
	const CanardCANFrame *out_frame;
	CanardCANFrame in_frame;
	CANRxFrame rxmsg;
	CANTxFrame txmsg;
	//int16_t rc;
	while(true) {
		if (canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
			chibiosCanardHandler(&in_frame, NULL, &rxmsg, NULL);
			canardHandleRxFrame(&m_canard_instance, &in_frame,
				TIME_I2MS(chVTGetSystemTimeX()));
		}

		out_frame = canardPeekTxQueue(&m_canard_instance);

		if (out_frame != NULL) { // If there are any frames to transmit
			chibiosCanardHandler(NULL, &*out_frame, NULL, &txmsg);
			if (canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100)) == MSG_OK) { // If transmit is successful
				canardPopTxQueue(&m_canard_instance);
			}
		}
	}
}

/** @brief Handles ActuatorCommand messages
 *
 * Updates position values according to stuff
 */
static void handle_actuator_command(CanardInstance *ins,
	CanardRxTransfer *transfer)
{
	uavcan_equipment_actuator_ArrayCommand msg;
	(void) ins;

	// Pull message data
	uavcan_equipment_actuator_ArrayCommand_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);

	for (int i = 0; i < msg.commands.len; i++) {
		uavcan_equipment_actuator_Command* cmd = &msg.commands.data[i];

		motor_set(cmd->command_value);
	}

}

struct can_msg_handler can_request_handlers[] = {
	CAN_MSG_HANDLER(UAVCAN_PROTOCOL_PARAM_GETSET_ID,
		UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE, handle_getSet),
	CAN_MSG_HANDLER(UAVCAN_PROTOCOL_GETNODEINFO_ID,
		UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE, return_node_info),
	CAN_MSG_HANDLER(UAVCAN_PROTOCOL_RESTARTNODE_ID,
		UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE, restart_node)
};

struct can_msg_handler can_broadcast_handlers[] = {
	CAN_MSG_HANDLER(UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID,
		UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_SIGNATURE, handle_actuator_command),
};

bool should_accept(const CanardInstance* ins,
					uint64_t* out_data_type_signature,
					uint16_t data_type_id,
					CanardTransferType transfer_type,
					uint8_t source_node_id)
{
	(void) source_node_id;
	(void) ins;


	if (transfer_type == CanardTransferTypeBroadcast) {
		for (uint16_t i = 0; i < NELEM(can_broadcast_handlers); i++) {
			if (data_type_id == can_broadcast_handlers[i].id) {
				*out_data_type_signature = can_broadcast_handlers[i].signature;
			}
		}
	}

	if (transfer_type == CanardTransferTypeRequest) {
		for (uint16_t i = 0; i < NELEM(can_request_handlers); i++) {
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
		for (uint16_t i = 0; i < NELEM(can_request_handlers); i++) {
			if (transfer->data_type_id == can_broadcast_handlers[i].id)
				can_broadcast_handlers[i].handler(ins,
					transfer);
		}
	}

	if (transfer->transfer_type == CanardTransferTypeRequest) {
		for (uint16_t i = 0; i < NELEM(can_request_handlers); i++) {
			if (transfer->data_type_id == can_request_handlers[i].id)
				can_request_handlers[i].handler(ins,
					transfer);
		}
	}
}

static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer)
{
	uavcan_protocol_RestartNodeRequest msg;
	(void) ins;

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
	out_msg.status.uptime_sec = TIME_I2S(chVTGetSystemTime());

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

void publish_nodeStatus(void) {
	uavcan_protocol_NodeStatus msg;
	uint16_t len;


	msg.health = node_health;
	msg.mode   = node_mode;
	msg.sub_mode = 0;
	msg.vendor_specific_status_code = 0;
	msg.uptime_sec = TIME_I2S(chVTGetSystemTime());

	len = uavcan_protocol_NodeStatus_encode(&msg, out_buf);

	canardBroadcast(&m_canard_instance,
			UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
			UAVCAN_PROTOCOL_NODESTATUS_ID,
			&inout_transfer_id,
			0,
			out_buf,
			len);
}
