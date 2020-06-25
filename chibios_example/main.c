/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#include <string.h>

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  while (true) {
    palClearPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
}

THD_TABLE_BEGIN
THD_TABLE_ENTRY(waThread1, "blinker", Thread1, NULL)
THD_TABLE_END

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  // Activate USART w/ custom config
  UARTConfig usart_config = {
    .txend1_cb   = NULL,
    .txend2_cb   = NULL,
    .rxend_cb    = NULL,
    .rxchar_cb   = NULL,
    .rxerr_cb    = NULL,
    .timeout_cb  = NULL,
    .timeout     = 0,
    .speed       = 9600
  };
  uartStart(&UARTD2, &usart_config);

  // main() thread activity, just outputs Hello World!
  while (true) {
    size_t bytes = strlen("Hello World!");
    uartSendTimeout(&UARTD2, (size_t*) &bytes, "Hello World!", 1000);
    chThdSleepMilliseconds(500);
  }
}
