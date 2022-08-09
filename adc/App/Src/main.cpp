#include "ch.h"
#include "hal.h"
#include "chprintf.h"
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
int main(void)
{
	chSysInit();
	halInit();
	palSetPadMode(GPIOA, GPIOA_PIN0, PAL_MODE_INPUT_ANALOG);
	sdStart(&SD2, NULL);
	adcStart(&ADCD1, NULL);
	while(1) {
		adcsample_t buffer[2];
		msg_t status = adcConvert(&ADCD1, &adcCfg, buffer, 2);
		chprintf((BaseSequentialStream*)&SD2, "ADC 0 is %d \r\n", buffer[0]);
		chThdSleepMilliseconds(20);
	}
}


