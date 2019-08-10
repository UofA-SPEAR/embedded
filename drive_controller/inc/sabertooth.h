/*
 * sabertooth.h
 *
 * Library to use sabertooth motor drivers (2x60A)
 */

#ifndef SABERTOOTH_H_
#define SABERTOOTH_H_

#include <stdint.h>

// Errors
typedef enum {
    SABER_SUCCESS,
    SABER_INVALID_VALUE,
} sabertooth_err_t;

// User function to write over serial
typedef void (*sabertooth_write8_t) (uint8_t data);

// Instance structure
typedef struct {
    sabertooth_write8_t write8;

    uint8_t address;
    uint8_t ramp_setting;
    uint8_t deadband;
    uint8_t timeout;

    float min_voltage;
    float max_voltage;
} sabertooth_t;


/** @brief Initialize sabertooth motor driver
 */
void sabertooth_init(sabertooth_t* saber);


/**@brief Function to set a motor speed.
 *
 * @param speed Signed integer representing speed to set motor at.
 *              Value > 0 represents forward motion and Value < 0 represents reverse motion.
 */
void sabertooth_set_motor(sabertooth_t* saber, uint8_t motor, int8_t speed);


/**@brief Function to set ramping rate of motor drivers
 *
 * @note Valid settings are from 1-80. See datasheet for reference on what values to send.
 */
sabertooth_err_t sabertooth_set_ramping(sabertooth_t* saber, uint8_t ramp_setting);


/**@brief Function to set deadband of motor drivers
 *
 * @note valid settings are 0-127
 */
sabertooth_err_t sabertooth_set_deadband(sabertooth_t* saber, uint8_t deadband);


/**@brief Function to set shutoff voltage from battery
 *
 * @note Voltage must be greater than 6v, and less than 30v, in increments of 0.2v
 */
sabertooth_err_t sabertooth_set_min_voltage(sabertooth_t* saber, float voltage);


/**@brief Function to set maximum voltage for the batteries
 *
 * @note Please don't use this I'm just putting it in for completeness.
 */
sabertooth_err_t sabertooth_set_max_voltage(sabertooth_t* saber, float voltage);

sabertooth_err_t sabertooth_set_timeout(sabertooth_t* saber, uint8_t timeout);

#endif // SABERTOOTH_H_
