#include "ch.h"
#include "hal.h"
#include "chprintf.h"



#define MV_PER_A 100.0

#define MAX_RAW_VALUE 3723.0

#define MAX_VOLTAGE 3000.0


static const ADCConversionGroup adcCfg = {
  .circular     = false,
  .num_channels = 4,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_CONT,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_19P5) | ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_19P5) | ADC_SMPR1_SMP_AN3(ADC_SMPR_SMP_19P5) | ADC_SMPR1_SMP_AN4(ADC_SMPR_SMP_19P5),
    0
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2) | ADC_SQR1_SQ3_N(ADC_CHANNEL_IN3) | ADC_SQR1_SQ4_N(ADC_CHANNEL_IN4), 
    0,
    0,
    0

  }
};




static const ADCConversionGroup adcCfg1 = {
  .circular     = false,
  .num_channels = 2,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_CONT,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_19P5) | ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_19P5),
    0
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
    0,
    0,
    0

  }
};




void read(adcsample_t test_val){

  adcsample_t buff1[4];
  adcsample_t buff2[2];
  msg_t status1 = adcConvert(&ADCD1, &adcCfg, buff1, 4);

  msg_t status2 = adcConvert(&ADCD2, &adcCfg, buff2, 2);

  float raw = (float)(buff1[0] + buff1[1])/2;

  float mv = raw*(MAX_VOLTAGE / MAX_RAW_VALUE) * 5.0/3.0;


  float amps;
  if (status1 == MSG_OK){
    amps = (mv - 2500.0)/MV_PER_A;
  } 
  else{
    amps = -1.0;
  }

  chprintf((BaseSequentialStream*)&SD2, "Current is %.2f \r\n", amps);


}


// static THD_WORKING_AREA(adcArea, 1024);

// static THD_FUNCTION(readFunc, arg){

//   int count = 0;

//   while(1){

//     read(count);
//     count++;

//     if (count == 4095){
//       count = 0;
//     }
//     chThdSleepMilliseconds(1);

//   }

// }




int main(void)
{
	chSysInit();
	halInit();
	sdStart(&SD2, NULL);


  palSetPadMode(GPIOA, GPIOA_PIN0, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_PIN1, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_USART2_TX, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_USART2_RX, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_PIN5, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT_ANALOG);


  adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);
  //(void) chThdCreateStatic(adcArea, sizeof(adcArea), NORMALPRIO, readFunc, NULL);

  int count = 1862;

  while(1){

    read(count);
    // count++;

    // if (count == 3723){
    //   count = 1862;
    // }
    chThdSleepMilliseconds(1);

  }


}


