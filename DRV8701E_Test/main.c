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
//#include <stdio.h>

#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      2

//static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
dacsample_t current;

static void pwmpcb(PWMDriver *pwmp) {
	(void)pwmp;
}

static void pwm4cb(PWMDriver *pwmp) {
	(void)pwmp;
}

static PWMConfig pwmcfg = {
	4000000, // 1MHz Timer Frequency
	500, // period is 500us to set the PWM signal to 80kHz
	pwmpcb,
	{
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_DISABLED,NULL},
		{PWM_OUTPUT_ACTIVE_HIGH,pwm4cb},
	},
	0,
	0
};

uint16_t adc_to_current(uint16_t sample)
{
	uint16_t ans;
	sample = sample*330/4095;
	ans = (sample - 5)/(10);
	return ans;
}

dacsample_t current_to_dac(dacsample_t current_wanted)
{
	dacsample_t res;
	res = current_wanted * 10 + 5;
	res *= (4095u/330u);
	return res;
}
void drv8701e_forward(uint32_t speed)
{
	palSetPad(GPIOB, GPIOB_PIN5);
	palSetPad(GPIOB, GPIOB_PIN8);
	pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, speed));
}

void drv8701e_backward(uint32_t speed)
{
	palSetPad(GPIOB, GPIOB_PIN5);
	palClearPad(GPIOB, GPIOB_PIN8);
	pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, speed));
}

void drv8701e_stop(void)
{
	palClearPad(GPIOB, GPIOB_PIN5);
}

static const DACConfig dac1cfg1 = {
	.init = 4047U,
	.datamode = DAC_DHRM_12BIT_RIGHT
};
/*
 * ADC streaming callback.
 */
//size_t nx = 0, ny = 0;
//static void adccallback(ADCDriver *adcp) {
//
//  if (adcIsBufferComplete(adcp)) {
//    nx += 1;
//  }
//  else {
//    ny += 1;
//  }
//}

//static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
//
//  (void)adcp;
//  (void)err;
//}
//
///*
// * ADC conversion group.
// * Mode:        Linear buffer, 2 samples of 1 channels, SW triggered.
// * Channels:    IN2.
// */
//static const ADCConversionGroup adcgrpcfg1 = {
//  FALSE,
//  ADC_GRP1_NUM_CHANNELS,
//  NULL,
//  adcerrorcallback,
//  ADC_CFGR_CONT,            /* CFGR    */
//  ADC_TR(0, 4095),          /* TR1     */
//  {                         /* SMPR[2] */
//    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_61P5),
//    0
//  },
//  {                         /* SQR[4]  */
//    ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
//    0,
//    0,
//    0
//  }
//};

/*
 * Red LEDs blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("current_control");
  while (true) {
	if(palReadPad(GPIOB, GPIOB_PIN14)) {
		palClearLine(LINE_LED);
	}
	else {
		palSetLine(LINE_LED);
	}
    chThdSleepMilliseconds(10);
  }
}

static THD_WORKING_AREA(waThread2, 512);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("drv8701_handler");
  while (true) {
	  drv8701e_forward(3000);
	  chThdSleepMilliseconds(5000);
	  drv8701e_stop();
	  chThdSleepMilliseconds(2000);
	  drv8701e_backward(3000);
	  chThdSleepMilliseconds(5000);
	  drv8701e_stop();
	  chThdSleepMilliseconds(2000);
  }
}

//static THD_WORKING_AREA(waThread3, 256);
//static THD_FUNCTION(Thread3, arg) {
//
//  (void)arg;
//  chRegSetThreadName("report_current");
//  while (true) {
//	  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
//	  uint16_t raw_data = 0;
//	  for(int i = 0; i < ADC_GRP1_BUF_DEPTH; i++) {
//	  		raw_data += samples1[i];
//	  }
//	  raw_data /= 2;
//	  char buffer[50];
//	  uint16_t report = adc_to_current(raw_data);
//	  uint8_t n = sprintf(buffer, "Current rn is: %u \r\n", report);
//	  sdWrite(&SD2, (uint8_t*)buffer, n);
//	  chThdSleepMilliseconds(500);
//  }
//}

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

  /*
   * Setting up pins.
   */
  palSetPadMode(GPIOA, GPIOA_PIN1, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, GPIOB_PIN8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, GPIOB_PIN9, PAL_MODE_ALTERNATE(2));


  /*
   * Creates the blinker thread.
   */

  /*
   * Activates the ADC1 driver and Serial2 Driver.
   */
	//  adcStart(&ADCD1, NULL);
	//  sdStart(&SD2, NULL);
  // palEnablePadEvent(GPIOB, GPIOB_PIN7, PAL_EVENT_MODE_FALLING_EDGE);

  // Start PWM and DAC
  pwmStart(&PWMD4, &pwmcfg);
  dacStart(&DACD1, &dac1cfg1);

  // enable TIM4 Channel 4
  pwmEnableChannel(&PWMD4, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 0));
  // set current limit at 1A
  current = current_to_dac(2u);
  dacPutChannelX(&DACD1, 0, current);

  // create 3 threads
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO + 1, Thread2, NULL);
//  chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);
  while (true) {
	  chThdSleepMilliseconds(500);
  }
}
