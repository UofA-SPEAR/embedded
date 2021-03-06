#ifndef COMS_H_
#define COMS_H_
#include "canard.h"


void comInit(void);
void updateComs(void);

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

// TODO check these priorities
#define CAN_GETSET_PRIORITY 30

// Should this go here or in another file?
// Paramater indexes to use
typedef enum {
	PARAMETER_motor1_actuator_id,
} parameter_id_t;

extern CanardInstance m_canard_instance;

extern uint64_t can_timestamp_usec;

extern uint8_t inout_transfer_id;

struct can_msg_handler {
	uint16_t id;
	uint64_t signature;
	void (*handler)(CanardInstance *ins, CanardRxTransfer *transfer);
};

#define CAN_MSG_HANDLER(topic_id, topic_signature, topic_handler) \
	{ \
		.id = topic_id, \
		.signature = topic_signature, \
		.handler = topic_handler \
	}

#define NELEM(a) (sizeof(a) / sizeof(*a))

// Public variables to set nodestatus
extern uint32_t node_health;
extern uint32_t node_mode;

bool should_accept(const CanardInstance* ins,
					uint64_t* out_data_type_signature,
					uint16_t data_type_id,
					CanardTransferType transfer_type,
					uint8_t source_node_id);

void on_reception(CanardInstance* ins, CanardRxTransfer* transfer);

int8_t tx_once(void);
int8_t rx_once(void);
int8_t handle_frame(void);
int16_t libcanard_init(CanardOnTransferReception on_reception,
		CanardShouldAcceptTransfer should_accept, void* user_reference,
		const uint32_t clock_rate, const uint32_t bitrate);
int16_t setup_hardware_can_filters(void);

void coms_handle_forever(void);

void publish_nodeStatus(void);


#endif
