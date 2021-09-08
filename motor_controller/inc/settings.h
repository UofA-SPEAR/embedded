/** @file settings.h
 *
 *  Created on: Feb 3, 2019
 *      Author: David lenfesty
 *
 *  Handles settings requests and the such coming in from CAN.
 *
 *  Assumes you know what you're doing when you set the values.
 *  If you don't, very bad things could happen and you should realise that.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "canard.h"
#include "uavcan/protocol/param/Value.h"

/** @brief Different data types that our settings can be
 */
typedef enum {
  SETTING_REAL = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE,
  SETTING_INTEGER = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE,
  SETTING_BOOLEAN = UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE,
} setting_type_t;

/** @brief Structure to hold the relevant information for settings
 */
struct setting_spec_t {
  char* name;
  setting_type_t union_tag;
};

struct setting_value_t {
  setting_type_t union_tag;
  union {
    double real;
    int64_t integer;
    bool boolean;
  } value;
};

/** @brief Standard initialisation for a real setting */
#define SETTING_SPEC_REAL(str) \
  { .name = str, .union_tag = SETTING_REAL }

/** @brief Standard initialisation for an integer setting */
#define SETTING_SPEC_INT(str) \
  { .name = str, .union_tag = SETTING_INTEGER }

/** @brief Standard initialisation for a boolean setting */
#define SETTING_SPEC_BOOL(str) \
  { .name = str, .union_tag = SETTING_BOOLEAN }

#define NUM_SETTINGS 18

extern const struct setting_spec_t setting_specs[NUM_SETTINGS];
extern struct setting_value_t saved_settings[NUM_SETTINGS];
extern struct setting_value_t pending_settings[NUM_SETTINGS];
extern struct setting_value_t current_settings[NUM_SETTINGS];

int8_t get_setting_index_by_name(char* name);
void handle_GetSet(CanardInstance* ins, CanardRxTransfer* transfer);

#endif /* SETTINGS_H_ */
