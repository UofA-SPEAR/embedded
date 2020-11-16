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

#include "canard.h"

#include <string.h>





#define RED1_PIN 6  //A6
#define BLUE1_PIN 7 //A7
#define GREEN1_PIN 0 //B0 (currently unused)

#define RED2_PIN 6 //B3
#define BLUE2_PIN 7 //B2
#define GREEN2_PIN 8 // B4 (currently unused)

#define RED1_CH 1
#define BLUE1_CH 2
#define GREEN1_CH 3

#define RED2_CH 1
#define BLUE2_CH 2
#define GREEN2_CH 3


int intensity = 1;

static PWMConfig cfg = {
  4000000,
  500,
  NULL,
  {

    {PWM_OUTPUT_ACTIVE_HIGH, NULL},

    {PWM_OUTPUT_ACTIVE_HIGH, NULL},

    {PWM_OUTPUT_ACTIVE_HIGH, NULL},

    {PWM_OUTPUT_ACTIVE_HIGH, NULL},

  },

  0,

  0

};







void setup(void){


  palSetPadMode(GPIOA, RED1_PIN, PAL_MODE_ALTERNATE(2));

  palSetPadMode(GPIOA, BLUE1_PIN, PAL_MODE_ALTERNATE(2));

  palSetPadMode(GPIOB, GREEN1_PIN, PAL_MODE_ALTERNATE(2));



  palSetPadMode(GPIOB, RED2_PIN, PAL_MODE_ALTERNATE(2));

  palSetPadMode(GPIOB, BLUE2_PIN, PAL_MODE_ALTERNATE(2));

  palSetPadMode(GPIOB, GREEN2_PIN, PAL_MODE_ALTERNATE(2));


  pwmStart(&PWMD3, &cfg);

  pwmStart(&PWMD4, &cfg);

}





void autonomy_pulse(void){

  pwmDisableChannel(&PWMD3, RED1_CH);
  pwmDisableChannel(&PWMD4, RED2_CH);

  pwmEnableChannel(&PWMD3, BLUE1_CH, (intensity++) % 500);
  pwmEnableChannel(&PWMD4, BLUE2_CH, (intensity++) % 500);

}



void drive_pulse(void){

  pwmDisableChannel(&PWMD3, BLUE1_CH);
  pwmDisableChannel(&PWMD4, BLUE2_CH);

  pwmEnableChannel(&PWMD3, RED1_CH, (intensity++) % 500);
  pwmEnableChannel(&PWMD4, RED2_CH, (intensity++) % 500);

}





// test function, delete when not needed anymore

void test(uint8_t ch){

  while(1){

    pwmEnableChannel(&PWMD3, ch, intensity++ % 500);

    //printf("%d", intensity);

    //palSetPad(GPIOB, 7);

    chThdSleepMilliseconds(1);

  }

} 





int main() { 

  halInit();

  chSysInit();

  setup();

  //Set enable pins to high
  //palSetPad(GPIOA, 2);
  //palSetPad(GPIOA, 3);

  // eventually this will be determined by the canbus, for now just hardcode and use it to 
  // select which leds to pulse
  int canbus_msg = 2;

  while (true) { 
    // continiously pulses blue or red, depending on what mode
    if (canbus_msg == NULL){
      continue;
    }

    else if (canbus_msg == 1){
      autonomy_pulse();
    }

    else if (canbus_msg == 2){
      drive_pulse();
    } 

    chThdSleepMilliseconds(5);
  }                                                                                                                                                               

}

