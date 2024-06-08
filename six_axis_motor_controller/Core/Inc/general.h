/**
  ******************************************************************************
  * @file           : General.h
  * @brief          : Header for main.c file.
  *                   This file defines parameters for the TMC5160 and EEPROM
  *                   SPI communication as well as general motor information.
  ******************************************************************************

  */
// The EERPROM instructions, stored as byte-sized constants, for the AT25512 EEPROM chip.
// Don't cares (bit-3) are set to zero.
#define EERPROM_INSTR_WREN 0b00000110; // STATUS Register Set Write Enable Latch (WEL)
#define EERPROM_INSTR_WRDI 0b00000100; // STATUS Register Reset Write Enable Latch (WEL)
#define EERPROM_INSTR_RDSR 0b00000101; // STATUS Register Read STATUS Register
#define EERPROM_INSTR_WRSR 0b00000001; // STATUS Register Write STATUS Register
#define EERPROM_INSTR_READ 0b00000011; // Memory Array Read from Memory Array
#define EERPROM_INSTR_WRIT 0b00000010; // Memory Array Write to Memory Array

// Constants relevant to the linear temperature sensor
#define ADC_TEMP_SLOPE 0.282 // The change of the ADC value with (0.322 calculated, 0.282 calibrated)
#define ADC_TEMP_INTER -47 // The ADC value if the temperature sensor is at 0 C (500 mV)
#define TEMP_RANGE_MAX 70 // The maximum temperature that the sensor can measure
#define TEMP_RANGE_MIN 0 // The minimum temperature that the sensor can measure
