/*
MIT License

Copyright (c) 2016 Mike Estee (Estee_TMC5160)
Copyright (c) 2017 Tom Magnier

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

#ifndef TMC5160_H
#define TMC5160_H

#define TMC5160_SPI_SIZE 5 // The size of the SPI datagrams in bytes.
#define BYTE_RESET 0x00 // The byte reset value for the SPI data lines.

#define CAN_MAIN_CONTR_ID 0x01 // The node ID of the main controller.
#define CAN_NO_DATA 0xFFFFFFFF // The no data frame for the CAN controller

#define CAN_DATA_SIZE 4 // The number of data bytes in the CAN data frames (float32 values).
// The Mask values for the CAN filter. Note, the 29 MSbits of the filter are the the CAN extended ID frame.
#define CAN_MASK_LOW 0x00008000 // The CAN Mask that selects for the board's ID.
#define CAN_MASK_HIGH 0x00000007
#define CAN_ID_LOW 0x00000000 // The generic ID of the CAN.
#define CAN_ID_HIGH 0x00000000

// The CAN motors/actuator IDs
#define CAN_MOTOR_ID_1 0x01
#define CAN_MOTOR_ID_2 0x02
#define CAN_MOTOR_ID_3 0x03
#define CAN_MOTOR_ID_4 0x04
#define CAN_MOTOR_ID_5 0x05
#define CAN_MOTOR_ID_6 0x06

#define MAX_SPEED_VAL 255

#include <TMC5160_registers.h>
#include <main.h>
#include <string.h>

namespace CAN_Command {

/* Register addresses */
enum {
    /* General configuration registers */
    GetPosition = 0x00, // Get the position of a motor
    GetSpeed = 0x01, // Get the speed of a motor
    SetPosition = 0x02, // Set the position of a motor
    SetSpeed = 0x03, // Set the speed of a motor
    HomeActuator = 0x04, // Home the actuators/motors
    Stop = 0x05, // Stop all motors
    Disable = 0x06, // Disable all drivers
	MoveWithSpeed = 0x07,
};

};

class TMC5160 {
public:
    static constexpr uint8_t IC_VERSION = 0x30;
    static constexpr uint32_t DEFAULT_F_CLK = 12000000; // Typical internal clock frequency in Hz.

    enum MotorDirection { NORMAL_MOTOR_DIRECTION = 0x00,
        INVERSE_MOTOR_DIRECTION = 0x1 };
    enum MotorType { STEPPER = 0x00,
        DC_BRUSHED = 0x01 };
    enum RampMode { POSITIONING_MODE,
        VELOCITY_MODE,
        HOLD_MODE };

    enum DriverStatus {
        OK, // No error condition
        CP_UV, // Charge pump undervoltage
        S2VSA, // Short to supply phase A
        S2VSB, // Short to supply phase B
        S2GA, // Short to ground phase A
        S2GB, // Short to ground phase B
        OT, // Overtemperature (error)
        OTHER_ERR, // GSTAT drv_err is set but none of the above conditions is found.
        OTPW // Overtemperature pre warning
    };

    struct PowerStageParameters {
        uint8_t drvStrength = 2; // MOSFET gate driver current (0 to 3)
        uint8_t bbmTime = 4; // "Break Before Make" duration specified in ns (0 to 24)
        uint8_t bbmClks = 4; // "Break Before Make" duration specified in clock cycles (0 to 15).
    };

    struct MotorParameters {
        uint16_t globalScaler = 32; // global current scaling (32 to 256)
        uint8_t irun = 16; // motor run current (0 to 31). For best performance don't set lower than 16
        uint8_t ihold = 0; // standstill current (0 to 31). Set 70% of irun or lower.
        TMC5160_Reg::PWMCONF_freewheel_Values freewheeling = TMC5160_Reg::FREEWHEEL_NORMAL; // Freewheeling / passive braking of ihold = 0
        uint8_t pwmOfsInitial = 30; // initial stealthChop PWM amplitude offset (0-255)
        uint8_t pwmGradInitial = 0; // initial stealthChop velocity dependent gradient for PWM amplitude
    };

    TMC5160(uint32_t fclk = DEFAULT_F_CLK);
    ~TMC5160();

    /* Start the motor driver using the specified parameters.
     * These should be tuned according to the power stage and motor used.
     * Look in the examples for a config wizard.
     * powerParams : power stage parameters
     * motorParams : motor current parameters
     * stepperDirection : normal / inverted
     */

    virtual bool begin(const PowerStageParameters& powerParams, const MotorParameters& motorParams, MotorDirection stepperDirection, MotorType mtrType /*=NORMAL_MOTOR_DIRECTION*/);
    void end();

    // TODO stealthChop tuning procedure

    virtual uint32_t readRegister(uint8_t address) = 0; // addresses are from TMC5160.h
    virtual uint8_t writeRegister(uint8_t address, uint32_t data) = 0;

    /* Check if the last register read was successful. This should be checked whenever
     a register read is used to take a decision.
     Reasons for failure can be : data bus disconnected, transmission error (bad CRC), etc
     This is mostly useful in UART mode.
     */
    bool isLastReadSuccessful();

    /* Ramp mode selection :
            - Positioning mode : autonomous move to XTARGET using all A, D and V parameters.
            - Velocity mode : follows VMAX and AMAX. Call setMaxSpeed() AFTER switching to velocity mode.
            - Hold mode : Keep current velocity until a stop event occurs.
    */
    void setRampMode(RampMode mode);

    float getCurrentPosition(); // Return the current internal position (steps)
    float getEncoderPosition(); // Return the current position according to the encoder counter (steps)
    float getLatchedPosition(); // Return the position that was latched on the last ref switch / encoder event (steps)
    float getLatchedEncoderPosition(); // Return the encoder position that was latched on the last encoder event (steps)
    float getTargetPosition(); // Get the target position (steps)
    float getDCSpeed(); // Get the set speed for in direct mode
    float getCurrentSpeed(); // Return the current speed (steps / second)

    void setCurrentPosition(float position, bool updateEncoderPos = false); // Set the current internal position (steps) and optionally update the encoder counter as well to keep them in sync.
    void setTargetPosition(float position); // Set the target position /!\ Set all other motion profile parameters before
    void setMaxSpeed(float speed); // Set the max speed VMAX (steps/second)
    void setTargetSpeed(float speed); // Set the target speed for DC motors in direct mode
    void setRampSpeeds(float startSpeed, float stopSpeed, float transitionSpeed); // Set the ramp start speed VSTART, ramp stop speed VSTOP, acceleration transition speed V1 (steps / second). /!\ Set VSTOP >= VSTART, VSTOP >= 0.1
    void setAcceleration(float maxAccel); // Set the ramp acceleration / deceleration (steps / second^2)
    void setAccelerations(float maxAccel, float maxDecel, float startAccel, float finalDecel); // Set the ramp accelerations AMAX, DMAX, A1, D1 (steps / second^2) /!\ Do not set finalDecel to 0 even if transitionSpeed = 0

    bool isTargetPositionReached(void); // Return true if the target position has been reached
    bool isTargetVelocityReached(void); // Return true if the target velocity has been reached

    bool isLeftLimitReached(void); // Return true if the left limit switch has been triggered
    bool isRightLimitReached(void); // Return true if the right limit switch has been triggered
    bool isMotorStalled(void);

    void stop(); // Stop the current motion according to the set ramp mode and motion parameters. The max speed and start speed are set to 0 but the target position stays unchanged.

    void disable(); // Disable the driver, all bridges off
    void enable(); // Enable the driver

    // TODO chopper config functions ?

    DriverStatus getDriverStatus(); // Get the current driver status (OK / error conditions)
    static const char* getDriverStatusDescription(DriverStatus st); // Get a human readable description of the given driver status

    /* Set the speeds (in steps/second) at which the internal functions and modes will be turned on or off.
     * Below pwmThrs, "stealthChop" PWM mode is used.
     * Between pwmThrs and highThrs, "spreadCycle" classic mode is used.
     * Between coolThrs and highThrs, "spreadCycle" is used ; "coolStep" current reduction and "stallGuard" load measurement can be enabled.
     * Above highThrs, "constant Toff" mode and fullstep mode can be enabled.
     * See the TMC 5160 datasheet for details and optimization.
     * Setting a speed to 0 will disable this threshold.
     */
    void setModeChangeSpeeds(float pwmThrs, float coolThrs, float highThrs);

    /* Set the encoder constant to match the motor and encoder resolutions.
     * This function will determine if the binary or decimal mode should be used
     * and return false if no exact match could be found (for example for an encoder
     * with a resolution of 360 and a motor with 200 steps per turn). In this case
     * the best approximation in decimal mode will be used.
     *
     * Params :
     * 		motorSteps : the number of steps per turn for the motor
     * 		encResolution : the actual encoder resolution (pulses per turn)
     * 		inverted : whether the encoder and motor rotations are inverted
     *
     * Return :
     * 		true if an exact match was found, false otherwise
     */
    bool setEncoderResolution(int32_t motorSteps, int32_t encResolution, bool inverted = false);

    /* Configure the encoder N event context.
     * Params :
     * 		sensitivity : set to one of ENCODER_N_NO_EDGE, ENCODER_N_RISING_EDGE, ENCODER_N_FALLING_EDGE, ENCODER_N_BOTH_EDGES
     * 		nActiveHigh : choose N signal polarity (true for active high)
     * 		ignorePol : if true, ignore A and B polarities to validate a N event
     * 		aActiveHigh : choose A signal polarity (true for active high) to validate a N event
     * 		bActiveHigh : choose B signal polarity (true for active high) to validate a N event
     */
    void setEncoderIndexConfiguration(TMC5160_Reg::ENCMODE_sensitivity_Values sensitivity, bool nActiveHigh = true, bool ignorePol = true, bool aActiveHigh = false, bool bActiveHigh = false);

    /* Enable/disable encoder and position latching on each encoder N event (on each revolution)
     * The difference between the 2 positions can then be compared regularly to check
     * for an external step loss.
     */
    void setEncoderLatching(bool enabled);

    /* Set maximum number of steps between internal position and encoder position
     * before triggering the deviation flag.
     * Set to 0 to disable. */
    void setEncoderAllowedDeviation(int steps);

    /* Check if a deviation between internal pos and encoder has been detected */
    bool isEncoderDeviationDetected();

    /* Clear encoder deviation flag (deviation condition must be handled before) */
    void clearEncoderDeviationFlag();

    // TODO end stops and stallguard config functions ?

    /* Configure the integrated short protection. Check datasheet for details.
     * - s2vsLevel : 4 (highest sensitivity) to 15 ; 6 to 8 recommended ; reset default 6
     * - s2gLevel : 2 (highest sensitivity) to 15 ; 6 to 14 recommended ; reset default 6 ; increase at higher voltage
     * - shortFilter : 0 to 3 ; reset default 1 ; increase in case of erroneous detection
     * - shortDelay : 0 to 1 ; reset default 0
     */
    void setShortProtectionLevels(int s2vsLevel, int s2gLevel, int shortFilter, int shortDelay = 0);

    void setStallProtectionLevels(int sgtLevels, int IstepUp, int IstepDwn, int minIval, int maxIval);

protected:
    static constexpr uint8_t WRITE_ACCESS = 0x80; // Register write access for spi / uart communication

    bool _lastRegisterReadSuccess = false;
    const uint32_t _fclk;

private:
    RampMode _currentRampMode;
    static constexpr uint16_t _uStepCount = 256; // Number of microsteps per step
    TMC5160_Reg::CHOPCONF_Register _chopConf = { 0 }; // CHOPCONF register (saved here to be restored when disabling / enabling driver)

    // Following §14.1 Real world unit conversions
    // v[Hz] = v[5160A] * ( f CLK [Hz]/2 / 2^23 )
    float speedToHz(int32_t speedInternal) { return ((float)speedInternal * (float)_fclk / (float)(1ul << 24) / (float)_uStepCount); }
    int32_t speedFromHz(float speedHz) { return (int32_t)(speedHz / ((float)_fclk / (float)(1ul << 24)) * (float)_uStepCount); }

    // Following §14.1 Real world unit conversions
    // a[Hz/s] = a[5160A] * f CLK [Hz]^2 / (512*256) / 2^24
    int32_t accelFromHz(float accelHz) { return (int32_t)(accelHz / ((float)_fclk * (float)_fclk / (512.0 * 256.0) / (float)(1ul << 24)) * (float)_uStepCount); }

    // See §12 Velocity based mode control
    int32_t thrsSpeedToTstep(float thrsSpeed) { return thrsSpeed != 0.0 ? (int32_t)constrain((float)_fclk / (thrsSpeed * 256.0), 0, 1048575) : 0; }
};

/* SPI interface : 
 * the TMC5160 SWSEL input has to be low (default state).
 */
class TMC5160_SPI : public TMC5160 {
public:
    SPI_HandleTypeDef* pspi; // A pointer to a SPI handle from the HAL.
    uint16_t chipSelectPin;
    GPIO_TypeDef* chipSelectPort; // A pointer to an IO port from the HAL.

    TMC5160_SPI(SPI_HandleTypeDef* h_spi, uint16_t Pin, GPIO_TypeDef* Port, uint32_t fclk = DEFAULT_F_CLK, uint8_t ID = 0);
    // An SPI handle pointer, the chip select pin and port

    uint32_t readRegister(uint8_t address); // addresses are from TMC5160.h
    uint8_t writeRegister(uint8_t address, uint32_t data);
    uint8_t readStatus();
    uint8_t CAN_MotorID; // The CAN actuator ID for this instance
    float CAN_MotorTxData; // The CAN data to send out.
    uint32_t CAN_MotorTxHeader; // The Tx Header for the motors to send out
    bool CAN_SendStatus; // true - needs to transmit, false - Nothing to send.
    void CAN_IN(CAN_RxHeaderTypeDef* pCAN_RxHeader, uint8_t RxData[CAN_DATA_SIZE]);

private:
    uint8_t send_array[TMC5160_SPI_SIZE];
    uint8_t rec_array[TMC5160_SPI_SIZE];
    void translateCAN(uint8_t commandID, uint8_t RxData[CAN_DATA_SIZE]);
    void motorCommand(uint8_t commandID, float CANfloatData);
    void _SPItransaction();
};

#endif // TMC5160_H
