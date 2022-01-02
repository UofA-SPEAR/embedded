/*
 * settings_parameters.c
 *
 *  Created on: Feb 3, 2019
 *      Author: isthatme
 */

#include "settings.h"

// Strings for all the parameters
const struct setting_spec_t setting_specs[NUM_SETTINGS] = {
    SETTING_SPEC_INT("spear.motor.firstboot"),
    SETTING_SPEC_BOOL("spear.motor.enabled"),
    SETTING_SPEC_INT("spear.motor.actuator_id"),
    SETTING_SPEC_BOOL("spear.motor.reversed"),
    SETTING_SPEC_BOOL("spear.motor.continuous"),
    SETTING_SPEC_REAL("spear.motor.pid.Kp"),
    SETTING_SPEC_REAL("spear.motor.pid.Ki"),
    SETTING_SPEC_REAL("spear.motor.pid.Kd"),
    SETTING_SPEC_INT("spear.motor.encoder.type"),
    SETTING_SPEC_INT("spear.motor.encoder.min"),
    SETTING_SPEC_INT("spear.motor.encoder.max"),
    SETTING_SPEC_REAL("spear.motor.encoder.to-radians"),
    SETTING_SPEC_INT("spear.motor.encoder.endstop_min"),
    SETTING_SPEC_INT("spear.motor.encoder.endstop_max"),
    SETTING_SPEC_REAL("spear.motor.linear.support_length"),
    SETTING_SPEC_REAL("spear.motor.linear.arm_length"),
    SETTING_SPEC_REAL("spear.motor.linear.length_min"),
    SETTING_SPEC_REAL("spear.motor.linear.length_max"),
};
