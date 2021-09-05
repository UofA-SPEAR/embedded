/*
 * settings.c
 *
 *  Created on: Feb 3, 2019
 *      Author: David Lenfesty
 *
 *
 *  General flow:
 *
 *  - Receives the GetSet request.
 *  - Decides whether to evaluate based on the name or the index (based on
 * whether there is a name)
 *  - Evaluates which index it should be (if done by ID, already done)
 *  - If there is data, it takes that data and stores it in flash, then responds
 * with that set value
 *  - If there isn't data, just responds with name of parameter, and current
 * value of parameter in flash
 *  - If any cases fail, just responds with an empty message. That signifies
 * there is no such parameter
 *
 *  Probably doesn't work right now.
 *  It does compile though, which is nice.
 */
#include "settings.h"

#include <stdlib.h>
#include <string.h>

#include "canard.h"
#include "coms.h"
#include "flash_settings.h"
#include "uavcan/protocol/param/GetSet.h"

static uint8_t dynamic_array_buf[300];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

extern uint8_t inout_transfer_id;

int8_t get_id_by_name(char* name) {
  for (int8_t i = 0; i < NUM_SETTINGS; i++) {
    // assumes strings are defined constant, thus have null termination
    if (strcmp(name, parameter_info[i].name) == 0) return i;
  }

  return -1;
}

/** @brief Reads and writes to settings.
 *
 */
void handle_getSet(CanardInstance* ins, CanardRxTransfer* transfer) {
  uavcan_protocol_param_GetSetRequest msg;
  uavcan_protocol_param_GetSetResponse resp;
  uint8_t resp_buf[100];
  char name_buf[UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE_MAX_LENGTH + 1];
  int8_t setting;

  // ignore messages that are too long
  if (transfer->payload_len > 300) {
    return;
  }

  uavcan_protocol_param_GetSetRequest_decode(transfer, transfer->payload_len,
                                             &msg, &p_dynamic_array_buf);

  if (msg.name.len > 0) {
    memcpy((void*)name_buf, (void*)msg.name.data, msg.name.len);
    name_buf[msg.name.len] = '\0';
    setting = get_id_by_name(name_buf);
  } else {
    setting = msg.index;
  }

  if (setting >= NUM_SETTINGS) {
    // Return empty data
    resp.name.len = 0;
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
  } else if (setting >= 0) {
    resp.name.len = strlen(parameter_info[setting].name);
    resp.name.data = (uint8_t*)parameter_info[setting].name;

    switch (parameter_info[setting].union_tag) {
      case (SETTING_REAL):
        resp.value.union_tag = SETTING_REAL;

        if (msg.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY)
          current_settings[setting].value.real = msg.value.real_value;

        resp.value.real_value = current_settings[setting].value.real;
        break;
      case (SETTING_INTEGER):
        resp.value.union_tag = SETTING_INTEGER;

        if (msg.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY)
          current_settings[setting].value.integer = msg.value.integer_value;

        resp.value.integer_value = current_settings[setting].value.integer;
        break;
      case (SETTING_BOOLEAN):
        resp.value.union_tag = SETTING_BOOLEAN;

        if (msg.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY)
          current_settings[setting].value.boolean = msg.value.boolean_value;

        resp.value.boolean_value = current_settings[setting].value.boolean;
        break;
      default:
        // TODO better error handling
        while (1)
          ;  // should never reach here
        break;
    }

    if (msg.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
      program_settings();
      load_settings();
    }
  } else {
    // Return empty data
    resp.name.len = 0;
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
  }

  resp.default_value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
  resp.max_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
  resp.min_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;

  uint8_t len = uavcan_protocol_param_GetSetResponse_encode(&resp, resp_buf);

  canardRequestOrRespond(
      ins, transfer->source_node_id, UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
      UAVCAN_PROTOCOL_PARAM_GETSET_ID, &inout_transfer_id, CAN_GETSET_PRIORITY,
      CanardResponse, (void*)resp_buf, len);
}
