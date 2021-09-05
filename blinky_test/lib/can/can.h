/*
 * can.h
 *
 *  Created on: Jul. 13, 2020
 *
 *
 */

#ifndef CAN_CAN_H_
#define CAN_CAN_H_

#include "o1heap.h"
#include "canard.h"

#ifndef CAN_XFER_QUEUE_SIZE
#define CAN_XFER_QUEUE_SIZE 5
#endif // CAN_MSG_QUE_SIZE

/// @brief Size of o1Heap memory pool to use (in bytes)
/// @note Must be multiple of sizeof(void*)
#ifndef CAN_O1HEAP_SIZE
#define CAN_O1HEAP_SIZE 1024
#endif // CAN_O1HEAP_SIZE

/// @brief Default UAVCAN heartbeat transmit period (ms)
/// @note Must be < 1000
#ifndef CAN_HEARTBEAT_PERIOD
#define CAN_HEARTBEAT_PERIOD 900
#endif

/// @brief Default periodic check timer (terrible explanation)
#ifndef CAN_PERIODIC_TIMER_PERIOD_US
#define CAN_PERIODIC_TIMER_PERIOD_US 500
#endif

/// @brief CAN Transmit timeout
#ifndef CAN_TX_TIMEOUT_US
#define CAN_TX_TIMEOUT_US 500
#endif

/// @brief Event ID to signal thread to check for outgoing frames.
#define CAN_TX_FRAME_EVT EVENT_MASK(1)
#define CAN_PERIODIC_TIMER_ID EVENT_MASK(2)
#define CAN_RX_FRAME_EVT EVENT_MASK(4)

#ifdef __cplusplus
extern "C" {
#endif

// -------- Static Config Checks -------- //
#include <assert.h>

static_assert(CAN_O1HEAP_SIZE % O1HEAP_ALIGNMENT == 0);
// Standard says heartbead freq must be > 1Hz
static_assert(CAN_HEARTBEAT_PERIOD <= 1000);


// -------- Configuration -------- //
typedef struct {
  CANDriver* can_driver;
  CANConfig* can_cfg;
  uint8_t node_id;
} CanardConfig;

// -------- Initialization -------- //
// Starts CAN threads runnning
CANConfig* get1MConfig(void);
/// @brief Initialises libcanard, and starts a thread to handle all CAN frames, which ID to use.
///
/// @returns A pointer to the CAN thread which handles sending out CAN frames
///
/// @note Call chEvtSignal(p_thread, CAN_TX_FRAME_EVT) to signal that frames
///       must be processed immediately.
///
/// @note Use
thread_t* canardStart(CanardConfig* cfg);

// -------- Transfer Management -------- //
// These functions are used to send and receive UAVCAN transfers.
// You should not do anything at the frame level, ever.

/// @brief Thread-safe accessor to send a transfer
/// @note no need to call canardAcquire
void canardSendTransfer(CanardTransfer* txfer);

/// @brief Blocking poll for incoming transfers.
CanardTransfer* canardAcceptTransfer(void);
/// @brief Must be called once finished with transfer to free data.
void canardFreeTransfer(CanardTransfer* xfer);

// -------- Mutex management -------- //
// Call these functions if you need to directly interface with the
// libcanard instance (e.g. subscribe to something)

/// @brief Locks the libcanard instance to interface with it.
CanardInstance* canardAcquire(void);
/// @brief Releases lock on libcanard instance
void canardRelease(CanardInstance* ins);

#ifdef __cplusplus
}
#endif





#endif /* CAN_CAN_H_ */
