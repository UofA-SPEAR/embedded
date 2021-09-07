#include "canard.h"
#include "main.h"
#include "can.h"

#include "ch.h"
#include "hal.h"

#include <stdbool.h>

CanardInstance canard_instance;
static uint8_t canard_memory_pool[LIBCANARD_MEM_POOL_SIZE];

/// @brief Copies received ChibiOS CAN frame into libcanard's required format
static void receive_canard_frame(CANRxFrame *in_frame, CanardCANFrame *out_frame) {
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
static void send_canard_frame(CanardCANFrame *in_frame, CANTxFrame *out_frame) {
  out_frame->IDE = CAN_IDE_EXT;
  out_frame->EID = in_frame->id & CANARD_CAN_EXT_ID_MASK;
  out_frame->RTR = CAN_RTR_DATA;
  out_frame->DLC = in_frame->data_len;
  for (uint8_t i = 0; i < in_frame->data_len; i++) {
    out_frame->data8[i] = in_frame->data[i];
  }
}


// Callbacks used on transfer acceptance.
static bool should_accept(const CanardInstance *ins, uint64_t *out_data_type_signature,
                   uint16_t data_type_id, CanardTransferType transfer_type,
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
void can_init(void) {
// TODO this needs to be device-specific
  const CANConfig config = {
      /*.mcr = */ CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
      /*.btr = */ CAN_BTR_SJW(1) | CAN_BTR_TS2(4) | CAN_BTR_TS1(5) |
          CAN_BTR_BRP(5)};
  canStart(&CAND1, &config);

  canardInit(&m_canard_instance, &canard_memory_pool,
             LIBCANARD_MEM_POOL_SIZE, on_reception, should_accept, NULL);
}

static bool restart_request;

/// @brief Takes control of thread to handle incoming and outgoing messages
void can_handle_forever(void) {
  const CanardCANFrame *out_frame;
  CanardCANFrame in_frame;
  CANTxFrame txmsg;
  CANRxFrame rxmsg;

  restart_request = false;

  while (true) {
    if (canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) ==
        MSG_OK) {
      receive_canard_frame(&rxmsg, &in_frame);
      canardHandleRxFrame(&m_canard_instance, &in_frame,
                          TIME_I2MS(chVTGetSystemTimeX()));
    }

    out_frame = canardPeekTxQueue(&m_canard_instance);

    if (out_frame != NULL) {  // If there are any frames to transmit
      send_canard_frame(out_frame, &txmsg);
      if (canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_IMMEDIATE) ==
          MSG_OK) {  // If transmit is successful
        canardPopTxQueue(&m_canard_instance);
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
  }
}

/// @brief Request a system restart once all messages are flushed.
void can_request_restart(bool reset) {
  restart_request = reset;
}
