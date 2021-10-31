#ifndef CAN_H_
#define CAN_H_

#include "canard.h"
#include "ch.h"
#include "hal.h"

// ---- Implementation Requirements ---- //
typedef struct {
  uint16_t id;
  uint64_t signature;
  void (*handler)(CanardInstance* ins, CanardRxTransfer* transfer);
} can_msg_handler;

#define CAN_MSG_HANDLER(topic_id, topic_signature, topic_handler) \
  { .id = topic_id, .signature = topic_signature, .handler = topic_handler }
#define CAN_MSG_HANDLER_END \
  { .id = 0, .signature = 0, .handler = NULL }

// ---- Usage functions ---- //

// Call this once during initialization
void can_init(CANConfig* hw_config, can_msg_handler broadcast_handlers[],
              can_msg_handler request_handlers[]);
// Place this in it's own idle thread to run forever
void can_handle_forever(void);
void can_request_restart(bool reset);
void can_set_node_status(uint8_t health, uint8_t mode);

/* ------------ Error Definitions --------------- */

#define LIBCANARD_SUCCESS 1
#define LIBCANARD_NO_QUEUE 0
#define LIBCANARD_ERR -1
#define LIBCANARD_ERR_NO_MEM -2
#define LIBCANARD_ERR_INVALID_SETTINGS -3
#define LIBCANARD_ERR_TX_QUEUE_FULL -4

/* ------------ Filtering Mask Definitions ------ */

#define CAN_MASK_UAVCAN_PRIORITY (0b11111 << 24)
#define CAN_MASK_UAVCAN_MSG_TYPE (0b1111111111111111 << 8)
#define CAN_MASK_UAVCAN_SRV_NOT_MSG (0b1 << 7)
#define CAN_MASK_UAVCAN_SRC_NODE_ID (0b1111111 << 0)

/* ------------ Value Definitions ---------------- */

// Memory pool size. Minimum is 1K.
#ifndef LIBCANARD_MEM_POOL_SIZE
#define LIBCANARD_MEM_POOL_SIZE 1024  // Default to 1K
#endif

// TODO check these priorities
#define CAN_GETSET_PRIORITY 30

#endif  // CAN_H_