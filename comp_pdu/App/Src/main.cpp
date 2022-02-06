#include "ch.h"
#include "hal.h"
#include "chprintf.h"



#define MV_PER_A 100.0

#define MAX_RAW_VALUE 4095.0

#define MAX_VOLTAGE 5000.0


static const ADCConversionGroup adcCfg = {
  .circular     = false,
  .num_channels = 1,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_CONT,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_19P5),
    0
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    0,
    0,
    0
  }
};


void read(adcsample_t test_val){

  //adcsample_t buff[2];
  msg_t status = adcConvert(&ADCD2, &adcCfg, &test_val, 2);


  float mv = (float)test_val*(MAX_VOLTAGE / MAX_RAW_VALUE);

  float amps;
  if (status == MSG_OK){
    amps = mv/MV_PER_A;
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
  adcStart(&ADCD2, NULL);
  palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT_ANALOG);

  //(void) chThdCreateStatic(adcArea, sizeof(adcArea), NORMALPRIO, readFunc, NULL);
  int count = 0;

  while(1){

    read(count);
    count++;

    if (count == 4095){
      count = 0;
    }
    chThdSleepMilliseconds(1);

  }


}


