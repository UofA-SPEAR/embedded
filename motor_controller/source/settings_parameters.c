/*
 * settings_parameters.c
 *
 *  Created on: Feb 3, 2019
 *      Author: isthatme
 */

#include "settings.h"

// Strings for all the parameters
const struct setting_spec_t setting_specs[NUM_SETTINGS] = {
    CAN_SETTING_INTEGER("spear.motor.firstboot"),
    CAN_SETTING_BOOLEAN("spear.motor.enabled"),
    CAN_SETTING_INTEGER("spear.motor.actuator_id"),
    CAN_SETTING_BOOLEAN("spear.motor.reversed"),
    CAN_SETTING_BOOLEAN("spear.motor.continuous"),
    CAN_SETTING_REAL("spear.motor.pid.Kp"),
    CAN_SETTING_REAL("spear.motor.pid.Ki"),
    CAN_SETTING_REAL("spear.motor.pid.Kd"),
    CAN_SETTING_INTEGER("spear.motor.encoder.type"),
    CAN_SETTING_INTEGER("spear.motor.encoder.min"),
    CAN_SETTING_INTEGER("spear.motor.encoder.max"),
    CAN_SETTING_REAL("spear.motor.encoder.to-radians"),
    CAN_SETTING_INTEGER("spear.motor.encoder.endstop_min"),
    CAN_SETTING_INTEGER("spear.motor.encoder.endstop_max"),
    CAN_SETTING_REAL("spear.motor.linear.support_length"),
    CAN_SETTING_REAL("spear.motor.linear.arm_length"),
    CAN_SETTING_REAL("spear.motor.linear.length_min"),
    CAN_SETTING_REAL("spear.motor.linear.length_max"),
};
