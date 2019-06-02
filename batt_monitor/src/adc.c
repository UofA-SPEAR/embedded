#include "adc.h"

ADC_HandleTypeDef hadc;

void adc_init(void) {
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode    = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion       = 2;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait      = DISABLE;    
    hadc.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    HAL_ADC_Init(&hadc);

    ADC_ChannelConfTypeDef channel;
    channel.Channel         = ADC_CHANNEL_1;
    channel.Rank            = ADC_REGULAR_RANK_1;
    channel.SamplingTime    = ADC_SAMPLETIME_61CYCLES_5;
    channel.SingleDiff      = ADC_SINGLE_ENDED;
    channel.OffsetNumber    = ADC_OFFSET_NONE;
    channel.Offset          = 0;
    HAL_ADC_ConfigChannel(&hadc, &channel);

    channel.Channel         = ADC_CHANNEL_3;
    channel.Rank            = ADC_REGULAR_RANK_2;
    HAL_ADC_ConfigChannel(&hadc, &channel);

    // Enable ADC
    hadc.Instance->CR |= ADC_CR_ADEN;
}

adc_measurement_t adc_measure(void) {
    adc_measurement_t out;

    // Start measurement
    hadc.Instance->CR |= ADC_CR_ADSTART;

    // Wait for data to be ready
    while (!(hadc.Instance->ISR & ADC_ISR_ADRDY));
    uint16_t current_measurement = hadc.Instance->DR;
    // 3v3 range, 1 / (20uOhm * 1000 gain)
    out.current = (current_measurement / 4096 * 3.3) * 50;

    while (!(hadc.Instance->ISR & ADC_ISR_ADRDY));
    uint16_t bat_voltage_measurement = hadc.Instance->DR;
    // 3v3 range, resistor divider by 10
    out.bat_voltage = (bat_voltage_measurement / 4096 * 3.3) * 10;

    return out;
}