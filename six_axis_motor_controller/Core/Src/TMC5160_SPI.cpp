/*
MIT License

Copyright (c) 2016 Mike Estee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "TMC5160.h"
#include <main.h>

TMC5160_SPI::TMC5160_SPI(SPI_HandleTypeDef* h_spi, uint16_t Pin, GPIO_TypeDef* Port, uint32_t fclk, uint8_t ID)
    : TMC5160(fclk)
{
    pspi = h_spi;
    chipSelectPin = Pin;
    chipSelectPort = Port;
    CAN_MotorID = ID;
    CAN_MotorTxData = 0;
    CAN_MotorTxHeader = 0x00010100 | (((uint32_t)(CAN_MotorID)) << 0);
    CAN_SendStatus = false;
}

void TMC5160_SPI::CAN_IN(CAN_RxHeaderTypeDef* pCAN_RxHeader, uint8_t RxData[CAN_DATA_SIZE])
{
    uint32_t rec_motorID = (pCAN_RxHeader->ExtId >> 8) & 0x0000000F;
    uint8_t commandID = (pCAN_RxHeader->ExtId >> 16) & 0x000000FF;
    if (rec_motorID == CAN_MotorID || commandID == CAN_Command::HomeActuator || commandID == CAN_Command::Stop) {
        translateCAN(commandID, RxData);
    }
    commandID = 0;
    rec_motorID = 0;
}

void TMC5160_SPI::translateCAN(uint8_t commandID, uint8_t RxData[CAN_DATA_SIZE])
{
    // CAN message is in the format extID (header), data1, data2, data3, data4
    // The 3 most significant bits are the motorID, followed by 5 bits
    // which are the controlID for what kind of command
    float CANfloatValue = 0;
    uint32_t CANdata = 0;

    CANdata |= (RxData[0]) << 24;
    CANdata |= (RxData[1]) << 16;
    CANdata |= (RxData[2]) << 8;
    CANdata |= (RxData[3]);

    CANfloatValue = reinterpret_cast<float&>(CANdata);

    motorCommand(commandID, CANfloatValue);

    CANfloatValue = 0;
    CANdata = 0;
    commandID = 0;
}

void TMC5160_SPI::motorCommand(uint8_t commandID, float CANfloatData)
{

    /* Takes in the command ID and converts values to set the motor's position
     * or velocity. The CAN_MotorTxData is adjusted in every case, either equal
     * to the required data or a NaN value.
     */

    CAN_SendStatus = true; // Indicates the motor has received an update.

    switch (commandID) {
    case CAN_Command::Stop:
        disable();
        CAN_MotorTxData = CAN_NO_DATA;
        break;

    case CAN_Command::HomeActuator: {
        setTargetPosition(0);
    }
        CAN_MotorTxData = CAN_NO_DATA;
        break;

    case CAN_Command::SetPosition:{
    	CANfloatData = CANfloatData / (M_TWOPI);
		CANfloatData = round(200 * CANfloatData);//Multiplied by steps per revolution
		setTargetPosition(CANfloatData);
		setMaxSpeed(stepperMaxSpeed);
		CAN_MotorTxData = getCurrentPosition();
		CAN_MotorTxHeader |= (CAN_Command::GetPosition << 16);
		wasSpeedMoving = false;
		break;
    }

    case CAN_Command::SetSpeed: { // Multiplying the float by the steps/second max speed
    	if (setMotorType == STEPPER){
    		if(CANfloatData == 0){
    			if (true){
    				wasSpeedMoving = false;
    				setMaxSpeed(stepperMaxSpeed);
    				//setCurrentPosition(getEncoderPosition(), false);
    				setTargetPosition(getCurrentPosition());
    			}
			} else {
				wasSpeedMoving = true;
				CANfloatData = constrain(CANfloatData, -1, 1);
				CANfloatData = round(stepperMaxSpeed * CANfloatData);
				setMaxSpeed(fabs(CANfloatData));
				if (CANfloatData > 0){
					setTargetPosition(1E6);
				} else if (CANfloatData < 0){
					setTargetPosition(-1E6);
				}
			}
    	} else if (setMotorType == DC_BRUSHED){
    		CANfloatData = CANfloatData * MAX_SPEED_VAL;
			setTargetSpeed(CANfloatData);
			CAN_MotorTxData = getCurrentSpeed();
			CAN_MotorTxHeader |= (CAN_Command::GetSpeed << 16);
    	}
    	break;
    }
    case CAN_Command::Disable: {
        disable();
    }
        CAN_MotorTxData = CAN_NO_DATA;
        break;
    case CAN_Command::MoveWithSpeed:{

	}
    default:
        CAN_MotorTxData = CAN_NO_DATA;
        break;
    }
    CANfloatData = 0;
    commandID = 0;
}

void TMC5160_SPI::_SPItransaction()
{
    // Setting chip-select pin low to enable communication.
    HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(pspi, send_array, rec_array, TMC5160_SPI_SIZE, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(chipSelectPort, chipSelectPin, GPIO_PIN_SET);
    // Setting the chip-select pin high to disable communication.
}

uint32_t TMC5160_SPI::readRegister(uint8_t address)
{
    // The register value to return.
    uint32_t value = 0;

    // Preparing the sending array to read the register
    send_array[0] = address;
    send_array[1] = BYTE_RESET;
    send_array[2] = BYTE_RESET;
    send_array[3] = BYTE_RESET;
    send_array[4] = BYTE_RESET;

    // Request the read for the address
    _SPItransaction();

    // Reading the register in the second cycle
    _SPItransaction();

    value |= (uint32_t)rec_array[1] << 24;
    value |= (uint32_t)rec_array[2] << 16;
    value |= (uint32_t)rec_array[3] << 8;
    value |= (uint32_t)rec_array[4];

    _lastRegisterReadSuccess = true; // In SPI mode there is no way to know if the TMC5130 is plugged...

    return value;
}

uint8_t TMC5160_SPI::writeRegister(uint8_t address, uint32_t data)
{
    // The value to return
    uint8_t status;

    // Setting up sending array
    send_array[0] = (address | WRITE_ACCESS); // Setting up for write access
    send_array[1] = ((data & 0xFF000000) >> 24);
    send_array[2] = ((data & 0xFF0000) >> 16);
    send_array[3] = ((data & 0xFF00) >> 8);
    send_array[4] = (data & 0xFF);

    // Processing the SPI communication
    _SPItransaction();

    status = rec_array[0];

    return status;
}

uint8_t TMC5160_SPI::readStatus()
{
    uint8_t status; // Declaring the status variable to be returned
    // Prepare the sending array using the GCONF address and dummy data
    send_array[0] = TMC5160_Reg::GCONF;
    send_array[1] = BYTE_RESET;
    send_array[2] = BYTE_RESET;
    send_array[3] = BYTE_RESET;
    send_array[4] = BYTE_RESET;

    _SPItransaction();

    // Setting the status value
    status = rec_array[0];

    return status;
}
