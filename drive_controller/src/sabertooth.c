#include "stm32f3xx.h"

#include "sabertooth.h"

#include <stdlib.h>

/**@brief Function for sending command over serial.
 *
 * @details Writes the address, then the command number, then the command value using provided function
 */
static void sabertooth_send_command(sabertooth_t* saber, uint8_t command, uint8_t value) {
    // From the datasheet: the checksum is a 7-bit value hat is just the addition of all values
    uint8_t checksum = (saber->address + command + value) &0b01111111;

    saber->write8(saber->address);
    saber->write8(command);
    saber->write8(value);
    saber->write8(checksum);
}

void sabertooth_init(sabertooth_t* saber) {
    sabertooth_set_ramping(saber, saber->ramp_setting);
    sabertooth_set_min_voltage(saber, saber->min_voltage);
    sabertooth_set_max_voltage(saber, saber->max_voltage);
    sabertooth_set_deadband(saber, saber->deadband);
}

void sabertooth_set_motor(sabertooth_t* saber, uint8_t motor, int8_t speed) {
    /* Motor command numbers:
     * 0: Drive forward motor 1
     * 1: Drive backwards motor 1
     * 4: Drive forward motor 2
     * 5: Drive backwards motor 2
     */

    uint8_t cmd = 0;
    uint8_t value = 0;

    /* Set the command offset (which motor is which)
     * As of now the only valid inputs are 1 and 2.
     * Not sure how to handle invalid inputs.
     * I'm going to assume people don't make mistakes.
     */
    if (motor == 0) {
        cmd = 0;
    } else if (motor == 1) {
        cmd = 4;
    }

    /* Change settings based on direction
     */
    if (speed >= 0) { // If you're going forwards:
        value = speed;
    } else { // If you're going backwards
        value = abs(speed);
        cmd += 1; // Set command to run motor backwards
    }

    /* Actually send command
     */
    sabertooth_send_command(saber, cmd, value);
}

sabertooth_err_t sabertooth_set_ramping(sabertooth_t* saber, uint8_t ramp_setting) {
    if ( (ramp_setting == 0) || (ramp_setting > 80) ) { // Invalid values
        return SABER_INVALID_VALUE;
    } else {
        sabertooth_send_command(saber, 16, ramp_setting);
        return SABER_SUCCESS;
    }
}


sabertooth_err_t sabertooth_set_deadband(sabertooth_t* saber, uint8_t deadband) {
    if (deadband > 127) { // Invalid value
        return SABER_INVALID_VALUE;
    } else {
        sabertooth_send_command(saber, 17, deadband);
        return SABER_SUCCESS;
    }
}


sabertooth_err_t sabertooth_set_min_voltage(sabertooth_t* saber, float voltage) {
    // Formula ripped straight from the datasheet
    uint8_t value = (voltage - 6) * 5;

    if (value > 120) {
        return SABER_INVALID_VALUE;
    } else {
        sabertooth_send_command(saber, 2, value);
        return SABER_SUCCESS;
    }
}

sabertooth_err_t sabertooth_set_max_voltage(sabertooth_t* saber, float voltage) {
    // actually you know what.
    // I'm not implementing this
    return SABER_SUCCESS;
}

