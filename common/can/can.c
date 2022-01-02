#include "can.h"

#include <stdbool.h>

#include "canard.h"
#include "uavcan/protocol/NodeStatus.h"

CanardInstance canard_instance;
static uint8_t canard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

static can_msg_handler *can_broadcast_handlers;
static can_msg_handler *can_request_handlers;

/// @brief Copies received ChibiOS CAN frame into libcanard's required format
static void receive_canard_frame(CANRxFrame *in_frame,
                                 CanardCANFrame *out_frame) {
  if (in_frame->IDE) {
    out_frame->id = CANARD_CAN_FRAME_EFF | in_frame->EID;
  } else {
    out_frame->id = in_frame->SID;
  }
  out_frame->data_len = in_frame->DLC;
  for (uint8_t i = 0; i < 8; i++) {
    out_frame->data[i] = in_frame->data8[i];
  }
}

/// @brief Copies Canard style frame into Chibios format
static void send_canard_frame(const CanardCANFrame *in_frame, CANTxFrame *out_frame) {
  out_frame->IDE = CAN_IDE_EXT;
  out_frame->EID = in_frame->id & CANARD_CAN_EXT_ID_MASK;
  out_frame->RTR = CAN_RTR_DATA;
  out_frame->DLC = in_frame->data_len;
  for (uint8_t i = 0; i < in_frame->data_len; i++) {
    out_frame->data8[i] = in_frame->data[i];
  }
}

// Callbacks used on transfer acceptance.
static bool should_accept(const CanardInstance *ins,
                          uint64_t *out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id) {
  (void)source_node_id;
  (void)ins;
  uint16_t i = 0;

  if (transfer_type == CanardTransferTypeBroadcast) {
    while (can_broadcast_handlers[i].handler != NULL) {
      if (data_type_id == can_broadcast_handlers[i].id) {
        *out_data_type_signature = can_broadcast_handlers[i].signature;
        return true;
      }

      i++;
    }
  }

  if (transfer_type == CanardTransferTypeRequest) {
    while (can_request_handlers[i].handler != NULL) {
      if (data_type_id == can_request_handlers[i].id) {
        *out_data_type_signature = can_request_handlers[i].signature;
        return true;
      }

      i++;
    }
  }

  return false;
}

static void on_reception(CanardInstance *ins, CanardRxTransfer *transfer) {
  uint16_t i = 0;

  if (transfer->transfer_type == CanardTransferTypeBroadcast) {
    while (can_broadcast_handlers[i].handler != NULL) {
      if (transfer->data_type_id == can_broadcast_handlers[i].id) {
        can_broadcast_handlers[i].handler(ins, transfer);
        break;
      }

      i++;
    }
  }

  if (transfer->transfer_type == CanardTransferTypeRequest) {
    while (can_request_handlers[i].handler != NULL) {
      if (transfer->data_type_id == can_request_handlers[i].id) {
        can_request_handlers[i].handler(ins, transfer);
        break;
      }

      i++;
    }
  }
}

/// @brief Initialize and start CAN device and libcanard instance.
///
/// @param[in] hw_config    Alternative HW configuration, NULL to use default
/// (known working for F303)
void can_init(CANConfig *hw_config, can_msg_handler broadcast_handlers[],
              can_msg_handler request_handlers[]) {
  const CANConfig default_config = {
      /*.mcr = */ CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
      /*.btr = */ CAN_BTR_SJW(1) | CAN_BTR_TS2(4) | CAN_BTR_TS1(5) |
          CAN_BTR_BRP(5)};

  if (hw_config != NULL) {
    canStart(&CAND1, hw_config);
  } else {
    canStart(&CAND1, &default_config);
  }

  if (broadcast_handlers == NULL || request_handlers == NULL) {
    chSysHalt("Misconfigured subscriptions!");
  }
  can_broadcast_handlers = broadcast_handlers;
  can_request_handlers = request_handlers;

  canardInit(&canard_instance, &canard_memory_pool, LIBCANARD_MEM_POOL_SIZE,
             on_reception, should_accept, NULL);
}

static bool restart_request;
static uint8_t node_health;
static uint8_t node_mode;
static uint8_t nodestatus_transfer_id;

static void publish_NodeStatus(void) {
  uavcan_protocol_NodeStatus msg;
  uint16_t len;

  msg.health = node_health;
  msg.mode = node_mode;
  msg.sub_mode = 0;
  msg.vendor_specific_status_code = 0;
  msg.uptime_sec = TIME_I2S(chVTGetSystemTime());

  len = uavcan_protocol_NodeStatus_encode(&msg, canard_memory_pool);

  canardBroadcast(&canard_instance, UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                  UAVCAN_PROTOCOL_NODESTATUS_ID, &nodestatus_transfer_id, 0,
                  canard_memory_pool, len);
}

/// @brief Takes control of thread to handle incoming and outgoing messages
void can_handle_forever(void) {
  const CanardCANFrame *out_frame;
  CanardCANFrame in_frame;
  CANTxFrame txmsg;
  CANRxFrame rxmsg;

  systime_t time = chVTGetSystemTimeX();
  restart_request = false;

  while (true) {
    if (canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) ==
        MSG_OK) {
      receive_canard_frame(&rxmsg, &in_frame);
      canardHandleRxFrame(&canard_instance, &in_frame,
                          TIME_I2MS(chVTGetSystemTimeX()));
    }

    out_frame = canardPeekTxQueue(&canard_instance);

    if (out_frame != NULL) {  // If there are any frames to transmit
      send_canard_frame(out_frame, &txmsg);
      if (canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_IMMEDIATE) ==
          MSG_OK) {  // If transmit is successful
        canardPopTxQueue(&canard_instance);
      }
    } else if (restart_request) {
      // We can restart now that we have sent all of our frames
      while (!(CAN->TSR & CAN_TSR_TME2))
        ;
      while (!(CAN->TSR & CAN_TSR_TME1))
        ;
      while (!(CAN->TSR & CAN_TSR_TME0))
        ;
      NVIC_SystemReset();
    }

    if (TIME_I2MS(time - chVTGetSystemTimeX()) > 1000) {
      publish_NodeStatus();
      time = chVTGetSystemTimeX();
    }
  }
}

/// @brief Request a system restart once all messages are flushed.
void can_request_restart(bool reset) { restart_request = reset; }

void can_set_node_status(uint8_t health, uint8_t mode) {
  node_health = health;
  node_mode = mode;
}