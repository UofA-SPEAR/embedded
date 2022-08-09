#include <stdlib.h>
#include "drv8701.hpp"

#define DRV8701_PH_MODE PAL_MODE_OUTPUT_PUSHPULL

#define DRV8701_EN_MODE PAL_MODE_ALTERNATE(2)

#define DRV8701_nFAULT_MODE PAL_MODE_INPUT_PULLUP

#define DRV8701_nSLEEP_MODE PAL_MODE_OUTPUT_PUSHPULL

#define DRV8701_SNSOUT_MODE PAL_MODE_INPUT_PULLUP

#define DRV8701_SO_MODE PAL_STM32_MODE_ANALOG

#define DRV8701_VREF_PORT GPIOA
#define DRV8701_VREF_PIN 4
#define DRV8701_VREF_MODE PAL_STM32_MODE_ANALOG

const DACConfig dac1cfg1 = {init : 4047U, datamode : DAC_DHRM_12BIT_RIGHT, {}};

void drv8701_init(const drv8701eControl_t *cfg) {
  PWMConfig pwmcfg = {
    4000000,  // 1MHz Timer Frequency
    500,      // period is 500us to set the PWM signal to 80kHz
    NULL,
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
  };
  pwmcfg.channels[cfg->channel] = cfg->channelCfg;
  palSetPadMode(DRV8701_VREF_PORT, DRV8701_VREF_PIN, DRV8701_VREF_MODE);
  palSetPadMode(cfg->gpioStruct.ph_pin.port, cfg->gpioStruct.ph_pin.pin, DRV8701_PH_MODE);
  palSetPadMode(cfg->gpioStruct.en_pin.port, cfg->gpioStruct.en_pin.pin, DRV8701_EN_MODE);
  palSetPadMode(cfg->gpioStruct.fault_pin.port, cfg->gpioStruct.fault_pin.pin, DRV8701_nFAULT_MODE);
  palSetPadMode(cfg->gpioStruct.sleep_pin.port, cfg->gpioStruct.sleep_pin.pin, DRV8701_nSLEEP_MODE);
  palSetPadMode(cfg->gpioStruct.snsout_pin.port, cfg->gpioStruct.snsout_pin.pin, DRV8701_SNSOUT_MODE);
  palSetPadMode(cfg->gpioStruct.so_pin.port, cfg->gpioStruct.so_pin.pin, DRV8701_SO_MODE);

  // Start PWM and DAC
  pwmStart(cfg->pwmDriver, &pwmcfg);
  dacStart(&DACD1, &dac1cfg1);

  // enable TIM4 Channel 4
  pwmEnableChannel(cfg->pwmDriver, cfg->channel,
                   PWM_PERCENTAGE_TO_WIDTH(cfg->pwmDriver, 0));
}

/// @brief Set speed, full range is +-10000
void drv8701_set(short velocity, const drv8701eControl_t *cfg) {
  uint16_t speed = abs(velocity);

  if (speed > 10000) speed = 10000;
  palSetPad(DRV8701_nSLEEP_PORT, DRV8701_nSLEEP_PIN);
  if (velocity > 0) {
    palSetPad(DRV8701_PH_PORT, DRV8701_PH_PIN);
    
    pwmEnableChannel(cfg->pwmDriver, cfg->channel,
                     PWM_PERCENTAGE_TO_WIDTH(cfg->pwmDriver, speed));
  } else {
    palClearPad(DRV8701_PH_PORT, DRV8701_PH_PIN);
    pwmEnableChannel(cfg->pwmDriver, cfg->channel,
                     PWM_PERCENTAGE_TO_WIDTH(cfg->pwmDriver, speed));
  }
}

void drv8701_stop(void) {
  palClearPad(DRV8701_nSLEEP_PORT, DRV8701_nSLEEP_PIN);
}

/// @brief Set chopping current of DRV8701.
///
/// @note This isn't a current limit being set, it actually just chops
///       at this level and coasts for a bit, so RMS current will be
///       somewhere below what is set.
///
/// @param[in] current	Current to set (in amps)
void drv8701_set_current(float current) {
  // write to DAC
  current = current * 10.0 + 5.0;
  current *= (4096.0 / 330.0);
  dacsample_t target = static_cast<dacsample_t>(current);
  dacPutChannelX(&DACD1, 0, target);
}
