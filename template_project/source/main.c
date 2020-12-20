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


static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  while (true) {
    //first thread
    chThdSleepMilliseconds(1);

  }
}

/**
 * setup function: initialize your stuff here!
 */
void setup(void) {
  __NOP();
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
   */
  halInit();
  chSysInit();
  sdStart(&SD2, NULL); // debug monitor, using PA2 and PA3
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL); 
  setup();
  // main() thread activity
  while (true) {
    //This is another thread
    chThdSleepMilliseconds(1);
  }
}
