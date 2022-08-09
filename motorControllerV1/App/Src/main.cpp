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
#include "chprintf.h"
#include "paramServer.hpp"
#include "coms.hpp"
#include "actuator.hpp"
__attribute__((weak)) void *__dso_handle;
// uint32_t __heap_base = 
/*
 * Application entry point.
 */
int main(void) {
  halInit();
  chSysInit();
  sdStart(&SD2, NULL);
  comsInit("spear.actuator", 16, 1000000);
  paramServerInit();
  uavcan::ParamServer server(getNode());
  data.init(&defaultCfg);
  int status = server.start(&data);
  chDbgAssert(status == 0, "server failed to start");
  motorInit();
  while (true) {
    getNode().spin(uavcan::MonotonicDuration::fromMSec(1));
    motorCheck();
    palToggleLine(LINE_LED);
  }
}
