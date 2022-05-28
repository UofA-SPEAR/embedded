#ifndef __DRV8701_H
#define __DRV8701_H

#include "hal.h"
#define DRV8701_SO_PORT GPIOA
#define DRV8701_SO_PIN 1
#define DRV8701_SNSOUT_PORT GPIOB
#define DRV8701_SNSOUT_PIN 14
#define DRV8701_nSLEEP_PORT GPIOA
#define DRV8701_nSLEEP_PIN 15
#define DRV8701_nFAULT_PORT GPIOB
#define DRV8701_nFAULT_PIN 7
#define DRV8701_EN_PORT GPIOB
#define DRV8701_EN_PIN 9
#define DRV8701_PH_PORT GPIOB
#define DRV8701_PH_PIN 8

typedef struct {
    struct {
        typedef struct {
            stm32_gpio_t *port;
            uint32_t pin;
        } gpio_t;
        gpio_t en_pin;
        gpio_t ph_pin;
        gpio_t fault_pin;
        gpio_t snsout_pin;
        gpio_t so_pin;
        gpio_t sleep_pin;
    } gpioStruct;
    PWMChannelConfig channelCfg;
    uint8_t channel;
    PWMDriver *pwmDriver;
    ADCDriver *adcDriver;
} drv8701eControl_t;

const drv8701eControl_t gpiover1_0 = {
    {
        {DRV8701_EN_PORT, DRV8701_EN_PIN},
        {DRV8701_PH_PORT, DRV8701_PH_PIN},
        {DRV8701_nFAULT_PORT, DRV8701_nFAULT_PIN},
        {DRV8701_SNSOUT_PORT, DRV8701_SNSOUT_PIN},
        {DRV8701_SO_PORT, DRV8701_SO_PIN},
        {DRV8701_nSLEEP_PORT, DRV8701_nSLEEP_PIN}
    },
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    3, // TIM4-CH4
    &PWMD4,
    &ADCD1,
};

void drv8701_init(const drv8701eControl_t *cfg);
void drv8701_set(short velocity, const drv8701eControl_t *cfg);
void drv8701_stop(void);
void drv8701_set_current(float current);

#endif  // DRV8701_H_