/*
 * can.c
 *
 *  Created on: Jul. 13, 2020
 *
 *
 */
#include "ch.h"
#include "hal.h"
#include "can.h"
#include "evtimer.h"
#include "canard_dsdl.h"

#include <string.h>

static mutex_t canard_mtx;
static CanardInstance canard;
static O1HeapInstance *o1heap;
static uint8_t canMailPool[CAN_XFER_QUEUE_SIZE * sizeof(CanardTransfer)];
static objects_fifo_t canardTransferMail;
static msg_t canardTransferMsg[CAN_XFER_QUEUE_SIZE];
__attribute__(( aligned(O1HEAP_ALIGNMENT) ))
static uint8_t o1heap_pool[CAN_O1HEAP_SIZE];

static CANConfig can1Mconfig = {
	/*.mcr = */ CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
	/*.btr = */ CAN_BTR_SJW(1) | CAN_BTR_TS2(4) |
	CAN_BTR_TS1(5) | CAN_BTR_BRP(5)
};

CANConfig* get1MConfig(void) {
  return &can1Mconfig;
}

void canardSendTransfer(CanardTransfer* txfer)
{
  chMtxLock(&canard_mtx);
  canardTxPush(&canard, txfer);
  chMtxUnlock(&canard_mtx);
}

THD_WORKING_AREA(waCanThread, 512);
THD_FUNCTION(CanThread, can)
{
  CANDriver* can_device = (CANDriver*) can;
  event_listener_t periodic_evt;
  event_listener_t rx_evt;
  event_timer_t evt;
  msg_t rc;

  // Set up events
  evtObjectInit(&evt, TIME_US2I(CAN_PERIODIC_TIMER_PERIOD_US));
  evtStart(&evt);
  chEvtRegisterMask(&evt.et_es, &periodic_evt, CAN_PERIODIC_TIMER_ID);
  chEvtRegisterMask(&(can_device->rxfull_event), &rx_evt, CAN_RX_FRAME_EVT);
  chEvtAddEvents(CAN_PERIODIC_TIMER_ID | CAN_TX_FRAME_EVT | CAN_RX_FRAME_EVT);
  while (1) {
    eventmask_t event = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE);
    if((event & CAN_PERIODIC_TIMER_ID) || (event & CAN_TX_FRAME_EVT)) { // if it reaches can periodic, proceed to send stuff
    	chMtxLock(&canard_mtx);
    	for (const CanardFrame* out_frame = NULL; (out_frame = canardTxPeek(&canard)) != NULL;)
    	{
    	    if (out_frame->timestamp_usec > TIME_I2US(chVTGetSystemTime()))
    	    {
    	    	CANTxFrame tx_frame = {
    	    			.DLC = CanardCANLengthToDLC[out_frame->payload_size],
				        .RTR = 0,
				        .IDE = 1,
				        .EID = out_frame->extended_can_id,
    	    	};
    	    	memcpy(&tx_frame.data8[0], &out_frame->payload, out_frame->extended_can_id);
    	        rc = canTransmitTimeout(can_device, CAN_ANY_MAILBOX, (const CANTxFrame*)&tx_frame, TIME_US2I(CAN_TX_TIMEOUT_US));
				if(rc == MSG_OK)
    	        {
    	    	    canardTxPop(&canard);
    	    	    canard.memory_free(&canard, (CanardFrame*)out_frame);
    	        }
    	        else {
    	        	//ERROR Handler
    	        }
    	    }

    	}
    	chMtxUnlock(&canard_mtx);
    }
    else if(event & CAN_RX_FRAME_EVT) { // receive event occurs
    	chMtxLock(&canard_mtx);
    	CANRxFrame rx_frame;
    	rc = canReceiveTimeout(can_device, CAN_ANY_MAILBOX, &rx_frame, TIME_IMMEDIATE);
    	if(rc == MSG_OK) {
    		CanardFrame raw_rx = {
    				.payload_size = CanardCANDLCToLength[rx_frame.DLC],
					.timestamp_usec = TIME_I2US(chVTGetSystemTime()),
					.extended_can_id = rx_frame.EID,
					.payload = (void*)&rx_frame.data8[0],
    		};
    		CanardTransfer transfer;
    		const int8_t result = canardRxAccept(&canard, &raw_rx, 0, &transfer);

    		/// we simply ignore result = 0 cuz it could be incomplete so just wait for canardRxAccept do its job
    		if(result > 0) {
    			CanardTransfer *pointer = chFifoTakeObjectTimeout(&canardTransferMail, TIME_INFINITE); // wait for free slot in mail
    			memcpy(pointer, &transfer, sizeof(CanardTransfer));
    			chFifoSendObject(&canardTransferMail, pointer);
    			// we can callback right here without using mailbox too :V
    		}
    		else if(result < 0) {
    			//ERROR handler
    		}
    	}
    	else {
    		//ERROR handler
    	}
    	chMtxUnlock(&canard_mtx);
    }
  }
}


THD_WORKING_AREA(waCanHeartbeatThread, 64);
THD_FUNCTION(CanHeartbeatThread, arg)
{
  uint8_t payload_buf[7];
  systime_t time;
  (void)arg;

  while (1) {
    time = chVTGetSystemTime();

    //////////////////////////////////////////////////////////////////////////////////////
    // this will be replaced by nunavut serialization soon
    canardDSDLSetUxx(&payload_buf[0],  0, TIME_I2S(time), 32);  // uptime
    canardDSDLSetUxx(&payload_buf[0], 32,          2,  2);      // health
    canardDSDLSetUxx(&payload_buf[0], 34,          2,  3);      // mode
    canardDSDLSetUxx(&payload_buf[0], 37,    0x7FFFF, 19);      // vssc
    //////////////////////////////////////////////////////////////////////////////////////
    CanardTransfer out = {
      .timestamp_usec = TIME_I2US(time) + CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
      .priority = 0,
      .port_id = 32085, // default heartbeat port id
      .remote_node_id = CANARD_NODE_ID_UNSET,
      .transfer_id = 0,
      .payload_size = 7,
      .payload = (void*) &payload_buf,
    };
    canardSendTransfer(&out);

    // Sleep until next transmit period
    chThdSleepUntil(time + TIME_MS2I(CAN_HEARTBEAT_PERIOD));
  }
}

CanardTransfer* canardAcceptTransfer(void) {
	CanardTransfer* pointer;
	chFifoReceiveObjectTimeout(&canardTransferMail, (void**)&pointer,TIME_INFINITE);
	return pointer;
}

void canardFreeTransfer(CanardTransfer* xfer) {
	canard.memory_free(&canard, (void*)xfer->payload);
	chFifoReturnObject(&canardTransferMail, xfer);
}

// replaced default heap to o1heap
static void* memAllocate(CanardInstance* const ins, const size_t amount)
{
    (void) ins;
    return o1heapAllocate(o1heap, amount);
}

static void memFree(CanardInstance* const ins, void* const pointer)
{
    (void) ins;
    o1heapFree(o1heap, pointer);
}

thread_t* canardStart(CanardConfig* cfg)
{
  // Mutex required to access libcanard instance
  chMtxObjectInit(&canard_mtx);
  // Initialize mailbox for incoming transfers

  // Outgoing transfer mailbox
  chFifoObjectInit(&canardTransferMail, sizeof(CanardTransfer), CAN_XFER_QUEUE_SIZE,
                   canMailPool, (void*)canardTransferMsg);

  o1heap = o1heapInit((void* const) o1heap_pool, CAN_O1HEAP_SIZE, (O1HeapHook)chSysLock, (O1HeapHook)chSysUnlock);
  canard = canardInit(&memAllocate, &memFree);
  canard.mtu_bytes = CANARD_MTU_CAN_CLASSIC;
  canard.node_id = cfg->node_id;
  canStart(cfg->can_driver, cfg->can_cfg);

  thread_t* can_thd = chThdCreateStatic(waCanThread, sizeof(waCanThread),
                                        LOWPRIO, CanThread, (void*)cfg->can_driver);
  (void) chThdCreateStatic(waCanHeartbeatThread, sizeof(waCanHeartbeatThread),
                           LOWPRIO, CanHeartbeatThread, NULL);
  return can_thd;
}

CanardInstance* canardAcquire(void)
{
  chMtxLock(&canard_mtx);
  return &canard;
}

void canardRelease(CanardInstance* ins)
{
  (void) ins;
  chMtxUnlock(&canard_mtx);
}
