#ifndef ADC_H_
#define ADC_H_

#include "stm32f3xx.h"

typedef struct {
    float current;
    float bat_voltage;
} adc_measurement_t;

/** @brief Initializes ADC peripheral with required settings.
 * 
 */
void adc_init(void);

/** @brief Reads board values
 * 
 */
adc_measurement_t adc_measure(void);

#endif