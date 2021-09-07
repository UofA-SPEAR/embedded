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

#include "main.h"

#include "can.h"

can_msg_handler can_broadcast_handlers[] = {CAN_MSG_HANDLER_END};

can_msg_handler can_request_handlers[] = {CAN_MSG_HANDLER_END};

static THD_WORKING_AREA(waHeartbeatThread, 128);
static THD_FUNCTION(HeartbeatThread, arg) {
  (void)arg;
  while (1) {
    // first thread
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(1000);
  }
}

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
   * - CAN initialization, starts the driver and initializes libcanard
   */
  halInit();
  chSysInit();
  can_init(NULL, can_broadcast_handlers, can_request_handlers);

  // Creates a new thread and runs it in it's own little stack space.
  chThdCreateStatic(waHeartbeatThread, sizeof(waHeartbeatThread), NORMALPRIO,
                    HeartbeatThread, NULL);

  // This is the main thread, it will continue to run even after we started
  // the other thread.

  // This function will now keep running until we reset.
  // NOTE: this function has a forever loop inside, otherwise we would
  // need to add a while (1) or something here if we wanted to keep this
  // main thread running.
  can_handle_forever();
}
