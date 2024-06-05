/*
 * EEPROM.cpp
 *
 *  Created on: Mar. 13, 2024
 *      Author: benja
 */

#include <main.h>
#include "EEPROM.h"

EEPROM_SPI::EEPROM_SPI(SPI_HandleTypeDef *h_spi, uint16_t CSN_Pin, GPIO_TypeDef *CSN_Port,
		uint16_t WP_Pin, GPIO_TypeDef *WP_Port, uint16_t HOLD_Pin, GPIO_TypeDef *HOLD_Port)
{
	pspi = h_spi;
	chipSelectPin = CSN_Pin;
	chipSelectPort = CSN_Port;
	writeProtectPin = WP_Pin;
	writeProtectPort = WP_Port;
	holdPin = HOLD_Pin;
	holdPort = HOLD_Port;

	//Disabling hold
	HAL_GPIO_WritePin(holdPort, holdPin, GPIO_PIN_SET);
	//Disabling write protect
	HAL_GPIO_WritePin(writeProtectPort, writeProtectPin, GPIO_PIN_SET);
}

void EEPROM_SPI::EEPROM_Write_Enable()
{
	//Write enable, must be done before all Write_Reg and Write_Status_reg
	//Disabling write protect to change the status register
	send_array[0] = EEPROM_Instr::WREN;//OPcode Write enable command 0x06
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);

}

void EEPROM_SPI::EEPROM_write(uint16_t address, uint32_t data)
{	// Writes to the EEPROM given a 16-bit starting address and 4-byte array

	uint8_t data_array[EEPROM_DATA_SIZE];
	data_array[0] = (uint8_t)((data & 0xFF000000) >> 24);
	data_array[1] = (uint8_t)((data & 0x00FF0000) >> 16);
	data_array[2] = (uint8_t)((data & 0x0000FF00) >> 8);
	data_array[3] = (uint8_t)(data & 0x000000FF);

	for(int i = 0; i < EEPROM_DATA_SIZE; i++)
	{
		HAL_Delay(10); //Delay 10 ms to allow the device to complete its internal operation
		EEPROM_Write_Enable();

		send_array[0] = EEPROM_Instr::WRIT;//OPcode WRITE command 0x02
		send_array[1] = (uint8_t)(((address + i) & 0xFF00) >> 8); //Write Addr high byte (A15- A8)
		send_array[2] = (uint8_t)((address + i) & 0x00FF); //Write Addr low byte (A7- A0)
		send_array[3] = data_array[i]; //Byte to be written to with data

		//Processing the SPI communication
		HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
		HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 4, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);
	}

}

uint32_t EEPROM_SPI::EEPROM_read(uint16_t address)
{
	uint32_t value = 0;

	for(int i = 0; i < EEPROM_DATA_SIZE; i++)
	{
		HAL_Delay(10); //Delay 10 ms to allow the device to complete its internal operation
		send_array[0] = EEPROM_Instr::READ;//OPcode READ command 0x03
		send_array[1] = (uint8_t)(((address + i) & 0xFF00) >> 8); //Addr high byte (A15- A8)
		send_array[2] = (uint8_t)((address + i) & 0x00FF); //Addr low byte (A7- A0)
		send_array[3] = 0;

		//Processing the SPI communication
		HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
		HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 4, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);

		value |= (uint32_t)rec_array[3] << (24 - 8*i);
	}

	return value;
}

uint8_t EEPROM_SPI::EEPROM_readStatus()
{
	//This function reads the status of the EEPROM.
	send_array[0] = EEPROM_Instr::RDSR; //OPcode READ command 0x05

	HAL_Delay(10); //Delay 10 ms to allow the device to complete its internal operation
	//Setting chip-select pin low to enable communication.
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);

	return rec_array[1];

}



void EEPROM_SPI::EEPROM_protect()
{
	EEPROM_Write_Enable();
	send_array[0] = EEPROM_Instr::WRSR;//OPcode Write Status Register command 0x01
	send_array[1] = 0x0C; //Write 0b00001100 bit 2 and 3 high for full memory protect, WPEN is low

	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);

}
void EEPROM_SPI::EEPROM_expose()
{
	EEPROM_Write_Enable();
	send_array[0] = EEPROM_Instr::WRSR;//OPcode Write Status Register command 0x01
	send_array[1] = 0x00; //Write 0b00000000 bit 2 and 3 high for full memory protect, WPEN is low

	//Processing the SPI communication
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(pspi, send_array, rec_array, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);

}


