/**
  ******************************************************************************
  * @file           : values.h
  * @brief          : Header for main.c file.
  *                   This file defines parameters for the adc temperature sensor.
  ******************************************************************************

  */

// Constants relevant to the linear temperature sensor
#define ADC_TEMP_SLOPE 0.282 //The change of the ADC value with (0.322 calculated, 0.282 calibrated)
#define ADC_TEMP_INTER -47 //The ADC value if the temperature sensor is at 0 C (500 mV)
#define TEMP_RANGE_MAX 70 //The maximum temperature that the sensor can measure
#define TEMP_RANGE_MIN 0 //The minimum temperature that the sensor can measure
#define TEMP_RANGE_FAN 50 //The fan start temperatue for the rover's electrical box




