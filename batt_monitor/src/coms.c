#include "coms.h"

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

uint32_t node_health	= UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
uint32_t node_mode		= UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE;

static void timestamp_tim_init(void);
static void restart_node(CanardInstance* ins, CanardRxTransfer* transfer);
static void return_node_info(CanardInstance* ins, CanardRxTransfer* transfer);


void coms_update(void) {
	tx_once();
	rx_once();
}

void coms_init(void) {
	timestamp_tim_init();
	libcanard_init(on_reception, should_accept, NULL, 64000000, 250000);
	setup_hardware_can_filters();
    // Configure interrupts
    // We only need to worry about the RX FIFO 0, because that's how the CAN interface is by default
    NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 1);
    NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
    CAN->IER |= 1 << CAN_IER_FMPIE0_Pos; // Enable CAN interrupt
}

bool should_accept(const CanardInstance* ins,
					uint64_t* out_data_type_signature,
					uint16_t data_type_id,
					CanardTransferType transfer_type,
					uint8_t source_node_id) {

	if (transfer_type == CanardTransferTypeBroadcast) {
		switch (data_type_id) {

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
		default:
			break;
		}
	}

	if (transfer->transfer_type == CanardTransferTypeRequest) {
		switch (transfer->data_type_id) {
		case(UAVCAN_PROTOCOL_PARAM_GETSET_ID):
			// TODO: Implement this
			// handle_getSet(ins, transfer);
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

	out_msg.name.len = strlen("Battery Monitor");
	out_msg.name.data = (uint8_t*) "Battery Monitor";
	out_msg.software_version.major = 0;
	out_msg.software_version.minor = 1;
	out_msg.hardware_version.major = 0;
	out_msg.hardware_version.minor = 1;
	out_msg.hardware_version.certificate_of_authenticity.len = 0;

	// TODO: hook this into main status
	out_msg.status.health 	= node_health;
	out_msg.status.mode 	= node_mode;
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
		canardHandleRxFrame(&m_canard_instance, &in_frame, can_timestamp_usec + TIM7->CNT);
		return LIBCANARD_SUCCESS;
	case 0:
		return LIBCANARD_NO_QUEUE;
	default:
		return LIBCANARD_ERR;
	}
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
	htim7.Init.Prescaler = 64;
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

// TODO: fill in the rest of the info.
void publish_batteryInfo(adc_measurement_t measurement, uint16_t status) {
	uavcan_equipment_power_BatteryInfo info;

	info.voltage 		= measurement.bat_voltage;
	info.current		= measurement.current;
	info.status_flags	= status;
	info.model_name.len = 0;

	uint16_t len = uavcan_equipment_power_BatteryInfo_encode(&info, out_buf);

	canardBroadcast(&m_canard_instance,
					UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE,
					UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID,
					&inout_transfer_id,
					0,
					out_buf,
					len);
}

int usleep(useconds_t usec) {
	useconds_t start = can_timestamp_usec + TIM7->CNT;
	while ((can_timestamp_usec + TIM7->CNT) < (start + usec));
}