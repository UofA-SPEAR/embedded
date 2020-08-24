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


#define RED1_PIN 7  //B7
#define BLUE1_PIN 8 //B8
#define GREEN1_PIN 6 //B6 (currently unused)

#define RED2_PIN 3 //A3
#define BLUE2_PIN 2 //A2
#define GREEN2_PIN 4 // A4 (currently unused)

#define RED1_CH 3
#define BLUE1_CH 1
#define GREEN1_CH 0

#define RED2_CH 3
#define BLUE2_CH 2
#define GREEN2_CH 1


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




  
  /*
  can funcions to be implemented
void can_init(void){


}

void get_frame(void){

} 
*/

void setup(void){


  palSetPadMode(GPIOB, RED1_PIN, PAL_MODE_ALTERNATE(10));
  palSetPadMode(GPIOB, BLUE1_PIN, PAL_MODE_ALTERNATE(10));
  palSetPadMode(GPIOB, GREEN1_PIN, PAL_MODE_ALTERNATE(5));

  palSetPadMode(GPIOA, RED2_PIN, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOA, BLUE2_PIN, PAL_MODE_ALTERNATE(1));
  palSetPadMode(GPIOA, GREEN2_PIN, PAL_MODE_ALTERNATE(2));

  pwmStart(&PWMD2, &cfg);
  pwmStart(&PWMD3, &cfg);
  pwmStart(&PWMD8, &cfg);
}


void autonomy_pulse(void){
  pwmDisableChannel(&PWMD3, RED1_CH);
  pwmDisableChannel(&PWMD2, RED2_CH);
  
  pwmEnableChannel(&PWMD8, BLUE1_CH, (intensity++) % 500);
  pwmEnableChannel(&PWMD2, BLUE2_CH, (intensity++) % 500);

}

void drive_pulse(void){
  pwmDisableChannel(&PWMD8, BLUE1_CH);
  pwmDisableChannel(&PWMD2, BLUE2_CH);

  pwmEnableChannel(&PWMD3, RED1_CH, (intensity++) % 500);
  pwmEnableChannel(&PWMD2, RED2_CH, (intensity++) % 500);
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
  //palSetPad(GPIOB, 9);
  //palSetPad(GPIOA, 1);

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
