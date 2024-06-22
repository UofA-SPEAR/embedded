/*
 * EEPROM.h
 *
 *  Created on: Mar. 13, 2024
 *      Author: benja
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#define EEPROM_DATA_SIZE 4 // The EEPROM data size in bytes, 2 for Addr (A15-A0), 1 for data

namespace EEPROM_Instr {
enum {
    WREN = 0b00000110, // STATUS Register Set Write Enable Latch (WEL)
    WRDI = 0b00000100, // STATUS Register Reset Write Enable Latch (WEL)
    RDSR = 0b00000101, // STATUS Register Read STATUS Register
    WRSR = 0b00000001, // STATUS Register Write STATUS Register
    READ = 0b00000011, // Memory Array Read from Memory Array
    WRIT = 0b00000010 // Memory Array Write to Memory Array
};

}

namespace EEPROM_Addr {
enum {
    MOT1 = 0x0100, // Motor 1 parameters MEM location
    MOT2 = 0x0104, // Motor 2 parameters MEM location
    MOT3 = 0x0108, // Motor 3 parameters MEM location
    MOT4 = 0x0112, // Motor 4 parameters MEM location
    MOT5 = 0x0116, // Motor 5 parameters MEM location
    MOT6 = 0x0120, // Motor 6 parameters MEM location
    GLOB_SCALE_MEM = 0x0124,
    IRUN_MEM = 0x0128,
    IHOLD_MEM = 0x0132,

};

}

#include <main.h>

class EEPROM_SPI {
public:
    SPI_HandleTypeDef* pspi; // A pointer to a SPI handle from the HAL.
    uint16_t chipSelectPin;
    GPIO_TypeDef* chipSelectPort; // A pointer to the chipselect port from the HAL.
    uint16_t writeProtectPin;
    GPIO_TypeDef* writeProtectPort;
    uint16_t holdPin;
    GPIO_TypeDef* holdPort;

    EEPROM_SPI(SPI_HandleTypeDef* h_spi, uint16_t CSN_Pin, GPIO_TypeDef* CSN_Port,
        uint16_t WP_Pin, GPIO_TypeDef* WP_Port, uint16_t HOLD_Pin, GPIO_TypeDef* HOLD_Port);
    /*An SPI handle pointer, the chip select pin and port, the write protect pin and port,
     * and the hold pin and port for the EEPROM.
     */
    void EEPROM_write(uint16_t address, uint32_t data);
    uint32_t EEPROM_read(uint16_t address);
    uint8_t EEPROM_readStatus();

private:
    uint8_t send_array[EEPROM_DATA_SIZE];
    uint8_t rec_array[EEPROM_DATA_SIZE];
    void EEPROM_Write_Enable();
    void EEPROM_protect();
    void EEPROM_expose();
};

#endif /* INC_EEPROM_H_ */
