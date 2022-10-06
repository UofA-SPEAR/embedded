#include "hal.h"
#include "slcan_shell.h"
#include "utils.h"
#include "string.h"

#define CAN_BUFF 100

typedef enum slcan_command_t{
  SLCAN_OPEN = 'O',
  SLCAN_CLOSE = 'C',
  SLCAN_SPEED = 'S',
  SLCAN_TRANSMIT_EID = 'T',
  SLCAN_TRANSMIT_SID = 't',
  SLCAN_RTR_EID = 'R',
  SLCAN_RTR_SID = 'r'
} slcan_command_t;

static CANTxFrame txframebuff[CAN_BUFF];
static msg_t txmsgbuff[CAN_BUFF];
static CANRxFrame rxframebuff[CAN_BUFF];
static msg_t rxmsgbuff[CAN_BUFF];


objects_fifo_t txfifo, rxfifo;

static mutex_t canmtx;

// event_source_t txstatus;
// event_listener_t txlistener;

// Obtained from http://www.bittiming.can-wiki.info/#STMicro
const uint32_t btr_lookup[] =
{
  0x011e00c7,
  0x011e0063,
  0x011e0027,
  0x011e0013,
  0x011e000f,
  0x011e0007,
  0x011e0003,
  0x011b0002,
  0x011e0001
};

uint8_t serialbuff[9 + 8 * 2 + 1];

static CANConfig cancfg = {
  .mcr = CAN_MCR_ABOM | CAN_MCR_AWUM,
  .btr = CAN_BTR_SJW(1) | CAN_BTR_TS2(4) | CAN_BTR_TS1(5) | CAN_BTR_BRP(5),
};

static bool isCanStopped(CANDriver *driver)
{
  return driver->state == CAN_STOP ? true: false;
}

static uint32_t get_nibbles(uint8_t *buff, int nibbles)
{
    int i;
    uint32_t id;
    char c;

    id = 0;
    for (i = 0; i < nibbles; i++) {
        c = buff[i];
        id <<= 4;
        id |= nibble2bin(c);
    }
    return id;
} 

static bool rtr_eid_assemble(objects_fifo_t *out_fifo, uint8_t *serialbuff)
{
  uint8_t buff[10];
  memcpy(buff, serialbuff, sizeof(buff));
  if(buff[9] != '\r') {
    return false;
  } 
  uint32_t id = 0;
  for(int i = 0; i < 8; i++) 
  {
    id <<= 4;
    id |= nibble2bin(buff[i]);
  }
  uint8_t length = nibble2bin(buff[8]);
  CANTxFrame *frame = chFifoTakeObjectTimeout(out_fifo, TIME_MS2I(1));
  if(frame == NULL) {
    return false;
  }
  frame->RTR = true;
  frame->DLC = length;
  frame->IDE = true;
  frame->EID = id;
  chFifoSendObject(out_fifo, frame);
  return true;
}

static bool rtr_sid_assemble(objects_fifo_t *out_fifo, uint8_t *serialbuff)
{
  uint8_t buff[5];
  memcpy(buff, serialbuff, sizeof(buff));
  if(buff[4] != '\r') {
    return false;
  } 
  uint32_t id = 0;
  for(int i = 0; i < 3; i++) 
  {
    id <<= 4;
    id |= nibble2bin(buff[i]);
  }
  uint8_t length = nibble2bin(buff[3]);
  CANTxFrame *frame = chFifoTakeObjectTimeout(out_fifo, TIME_MS2I(1));
  if(frame == NULL) {
    return false;
  }
  frame->RTR = true;
  frame->DLC = length;
  frame->IDE = false;
  frame->SID = id;
  chFifoSendObject(out_fifo, frame);
  return true;
}

static bool transmit_sid_assemble(objects_fifo_t *out_fifo, uint8_t *serialbuff)
{
  uint8_t buff[4 + 8 * 2 + 1];
  memcpy(buff, serialbuff, sizeof(buff));
  uint32_t length = nibble2bin(buff[3]);
  uint32_t id = 0;
  for(int i = 0; i < 3; i++) 
  {
    id <<= 4;
    id |= nibble2bin(buff[i]);
  }
  CANTxFrame *frame = chFifoTakeObjectTimeout(out_fifo, TIME_MS2I(1));
  if(frame == NULL) {
    return false;
  }
  frame->RTR = false;
  frame->DLC = length;
  frame->IDE = false;
  frame->SID = id;
  for(uint8_t i = 0; i < length; i++) 
  {
    frame->data8[i] = get_nibbles(&buff[4 + i * 2], 2);
  }
  chFifoSendObject(out_fifo, frame);
  return true;
}

static bool transmit_eid_assemble(objects_fifo_t *out_fifo, uint8_t *serialbuff)
{
  uint8_t buff[9 + 8 * 2 + 1];
  memcpy(buff, serialbuff, sizeof(buff));
  uint32_t length = nibble2bin(buff[8]);
  uint32_t id = 0;
  for(int i = 0; i < 8; i++) 
  {
    id <<= 4;
    id |= nibble2bin(buff[i]);
  }
  CANTxFrame *frame = chFifoTakeObjectTimeout(out_fifo, TIME_MS2I(1));
  if(frame == NULL) {
    return false;
  }
  frame->RTR = false;
  frame->DLC = length;
  frame->IDE = true;
  frame->EID = id;
  for(uint8_t i = 0; i < length; i++) 
  {
    frame->data8[i] = get_nibbles(&buff[9 + i * 2], 2);
  }
  chFifoSendObject(out_fifo, frame);
  return true;
}

static void transmit_received_frame(BaseChannel *stream, CANRxFrame *rxframe)
{
  uint8_t buff[9 + 8 * 2 + 1];
  uint8_t index = 0;
  // Extended Frame
  if(rxframe->IDE) {
    if(rxframe->RTR) {
      buff[index] = 'R';
    }
    else {
      buff[index] = 'T';
    }
    uint32_t id = rxframe->EID;
    uint8_t c = (id >> 24) & 0xff;
    index++;
    bin2hex(&buff[index], c);
    index += 2;
    c = (id >> 16) & 0xff;
    bin2hex(&buff[index], c);
    index += 2;
    c = (id >> 8) & 0xff;
    bin2hex(&buff[index], c);
    index += 2;
    c = id & 0xff;
    bin2hex(&buff[index], c);
    index += 2;
    c = (rxframe->DLC & 0xf) | 0x30;
    buff[index] = c;
    index++;
  }
  else {
    if(rxframe->RTR) {
      buff[index] = 'r';
    }
    else {
      buff[index] = 't';
    }
    index++;
    uint32_t id = rxframe->SID;
    uint8_t c = (id >> 8) & 0x07;
    c += 0x30;
    buff[index] = c;
    index++;
    c = id & 0xff;
    bin2hex(&buff[index], c);
    index += 2;
    c = (rxframe->DLC & 0xf) | 0x30;
    buff[index] = c;
    index++;
  }
  if(!rxframe->RTR) {
    for(uint8_t i = 0; i < rxframe->DLC; i++) {
      uint8_t c = rxframe->data8[i];
      bin2hex(&buff[index], c);
      index += 2;
    }
  }
  buff[index] = '\r';
  index++;
  size_t size = chnWriteTimeout(stream, buff, index, TIME_MS2I(100));
  chDbgAssert(size == index, "Error Happens");
}

void sendError(BaseChannel *stream)
{
  chnPutTimeout(stream, '\a', TIME_INFINITE);
}

void sendSuccess(BaseChannel *stream)
{
  chnPutTimeout(stream, '\r', TIME_INFINITE);
}

static void can_rx_cb(CANDriver *canp, uint32_t flags)
{
  CANRxFrame frame;
  chSysLockFromISR();
  bool isg = canTryReceiveI(canp, CAN_ANY_MAILBOX, &frame);
  if(!isg) {
    CANRxFrame *rxframe = chFifoTakeObjectI(&rxfifo);
    if(rxframe != NULL) {
      memcpy(rxframe, &frame, sizeof(frame));
      chFifoSendObjectI(&rxfifo, rxframe);
    }
    
  }
  chSysUnlockFromISR();
}

static THD_WORKING_AREA(waCanThread, 128);
static THD_FUNCTION(CanFunc, arg) {
  (void)arg;
  while(1) {
    chMtxLock(&canmtx);
    if(CAND1.state == CAN_READY) {
      CANTxFrame *txframe = NULL;
      msg_t msg = chFifoReceiveObjectTimeout(&txfifo, (void**)&txframe, TIME_MS2I(100));
      if(msg == MSG_OK) {
        msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, txframe, TIME_MS2I(100));
        if(msg == MSG_OK) {
          palToggleLine(LINE_LED);
        }
        chFifoReturnObject(&txfifo, txframe);
      }
      chMtxUnlock(&canmtx);
      CANRxFrame *rxframe;
      msg = chFifoReceiveObjectTimeout(&rxfifo, (void**)&rxframe, TIME_MS2I(100));
      if(msg == MSG_OK) {
        palToggleLine(LINE_LED);
        transmit_received_frame((BaseChannel*)&SD2, rxframe);
        chFifoReturnObject(&rxfifo, rxframe);
      }
    }
    else {
      chMtxUnlock(&canmtx);
      chThdSleepMilliseconds(100);
    }
  }
}

typedef enum slcan_state {
  CLOSE,
  OPEN
} slcan_state_t;

static THD_WORKING_AREA(waCanShellThread, 1024);
static THD_FUNCTION(CanShellFunc, arg) {
  BaseChannel *stream = (BaseChannel *)arg;
  uint8_t index = 0;
  uint8_t mode = 0;
  slcan_state_t state = CLOSE;
  while(1) {
    int chara = 0;
    while(chara != '\r') {
      chara = streamGet(stream);
      serialbuff[index] = (char)chara;
      index++;
    } 
    uint8_t c = serialbuff[0];
    if(state == CLOSE && chara == '\r') {
      switch((slcan_command_t)c) {
        case SLCAN_SPEED:
          if(isCanStopped(&CAND1)) {
            uint8_t check[2];
            memcpy(check, &serialbuff[1], 2);
            if(index != 3) {
              sendError(stream);
            }
            else {
              mode = nibble2bin(check[0]);
              if(index > 8) {
                sendError(stream);
              }
              else {
                sendSuccess(stream);
              }
            }
          }
          break; 
        case SLCAN_OPEN:
          if(isCanStopped(&CAND1)) {
            cancfg.btr = btr_lookup[mode];
            canStart(&CAND1, &cancfg);
            state = OPEN;
            sendSuccess(stream);
          }
          break;
        default:
          // sendError(stream);
          break;
      }
    }
    else if(state == OPEN && chara == '\r') {
      switch((slcan_command_t)c) {
        case SLCAN_CLOSE:
            chMtxLock(&canmtx);
            canStop(&CAND1);
            chMtxUnlock(&canmtx);
            sendSuccess(stream);
            state = CLOSE;
          break;
        case SLCAN_RTR_EID:
          if(!rtr_eid_assemble(&txfifo, &serialbuff[1])) {
            sendError(stream);
          }
          else {
            chnPutTimeout(&SD2, 'Z', TIME_MS2I(100));
            sendSuccess(stream);
          }
          break;
        case SLCAN_RTR_SID:
          if(!rtr_sid_assemble(&txfifo, &serialbuff[1])) {
            sendError(stream);
          }
          else {
            chnPutTimeout(&SD2, 'z', TIME_MS2I(100));
            sendSuccess(stream);
          }
          break;
        case SLCAN_TRANSMIT_EID:
          if(!transmit_eid_assemble(&txfifo, &serialbuff[1])) {
            sendError(stream);
          }
          else {
            chnPutTimeout(&SD2, 'Z', TIME_MS2I(100));
            sendSuccess(stream);
          }
          break;
        case SLCAN_TRANSMIT_SID:
          if(!transmit_sid_assemble(&txfifo, &serialbuff[1])) {
            sendError(stream);
          }
          else {
            chnPutTimeout(&SD2, 'z', TIME_MS2I(100));
            sendSuccess(stream);
          }
          break;
        default:
            sendError(stream);
            break;
      }
    }
    if(chara == '\r') {
      index = 0;  
    }
  }
}

void canShellInit(void)
{
  // chEvtRegisterMaskWithFlags(&txstatus, &txlistener, EVENT_MASK(0), SLCAN_TX_ERROR | SLCAN_TX_OK);
  chMtxObjectInit(&canmtx);
  CAND1.rxfull_cb = can_rx_cb;
  chFifoObjectInit(&txfifo, sizeof(CANTxFrame), CAN_BUFF, txframebuff, txmsgbuff);
  chFifoObjectInit(&rxfifo, sizeof(CANRxFrame), CAN_BUFF, rxframebuff, rxmsgbuff);
  chThdCreateStatic(waCanShellThread, sizeof(waCanShellThread), 
                    HIGHPRIO, CanShellFunc, &SD2);  
  chThdCreateStatic(waCanThread, sizeof(waCanThread), 
                  NORMALPRIO, CanFunc, NULL);  
}