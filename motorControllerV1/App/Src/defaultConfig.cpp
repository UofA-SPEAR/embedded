#include "paramServer.hpp"
const cfgArray_t defaultCfg = {{
    SETTING_SPEC_BOOL("enabled", false),
    SETTING_SPEC_INT("actuator_id", 41, 0, 255),
    SETTING_SPEC_BOOL("reversed", false),
    SETTING_SPEC_BOOL("continuous", false),
    SETTING_SPEC_REAL("pid.Kp", 0.0, -1.0, 1.0),
    SETTING_SPEC_REAL("pid.Ki", 0.0, -1.0, 1.0),
    SETTING_SPEC_REAL("pid.Kd", 0.0, -1.0, 1.0),
    SETTING_SPEC_INT("encoder.type", 0, 0, 3),
    SETTING_SPEC_INT("encoder.min", 0, -1000, 1000),
    SETTING_SPEC_INT("encoder.max", 0, -1000, 1000),
    SETTING_SPEC_INT("quadrature.gear", 0, 150, 5000),
    SETTING_SPEC_INT("encoder.endstop_min", 0, -1000, 1000),
    SETTING_SPEC_INT("encoder.endstop_max", 0, -1000, 1000),
    SETTING_SPEC_REAL("current", 5.0, 0, 20.0),
}};
