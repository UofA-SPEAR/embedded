#ifndef SLCAN_SHELL_H_
#define SLCAN_SHELL_H_

#define SLCAN_TX_OK     (1 << 1)
#define SLCAN_TX_ERROR  (1 << 2)
// bool isCanStopped();
extern objects_fifo_t txfifo, rxfifo;
void canShellInit(void);

#endif