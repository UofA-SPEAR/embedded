/*
 * settings.c
 *
 *  Created on: Feb 3, 2019
 *      Author: isthatme
 */
#include <stdlib.h>
#include <string.h>

#include "settings.h"

#include "uavcan/protocol/param/GetSet.h"

static uint8_t dynamic_array_buf[300];
static uint8_t* p_dynamic_array_buf = dynamic_array_buf;

extern uint8_t inout_transfer_id;

// Parameters can be found in settings_parameters.c
extern char* parameters[NUM_PARAMETERS];


void parameter_respond_from_id(CanardInstance* ins, uint16_t index) {
	// TODO respond to parameter requests without names
}

/** @brief Reads and writes to settings.
 *
 */
void handle_getSet(CanardInstance* ins, CanardRxTransfer* transfer) {
	if (transfer->payload_len > 300) { //ignore messages that are too long
		return;
	}

	uavcan_protocol_param_GetSetRequest msg;
	char name_buf[92]; // max length of name is 92 bytes

	uavcan_protocol_param_GetSetRequest_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);

	if (msg.name.len > 0) {
		// Copy name into temporary buffer
		memcpy(name_buf, msg.name.data, msg.name.len);

		if (strcmp(name_buf, "spear.arm") == 0) { // ignore stuff not in the right namespace

			if (strcmp(name_buf, parameter_motorA_actuator_id_name) == 0) {
				if (msg.value.union_tag == UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
					// return current value
					uavcan_protocol_param_GetSetResponse response;
					uint8_t out_buf[40];

					response.value.integer_value = actuator_id;
					response.default_value.integer_value = 10;
					response.name.data = (uint8_t *) parameter_motor1_actuator_id_name;
					response.name.len = strlen(parameter_motor1_actuator_id_name);

					uint8_t len = uavcan_protocol_param_GetSetResponse_encode(&response, out_buf);

					canardBroadcast(ins,
							UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
							UAVCAN_PROTOCOL_PARAM_GETSET_ID,
							&inout_transfer_id,
							30, // TODO verify this priority
							out_buf,
							len);

				} else {
					// set actuator id
					actuator_id = msg.value.integer_value;
					// TODO implement flash writing/reading
				}
			}
		}
	} else {
		// No defined behaviour when setting parameter with ID
		parameter_respond_from_id(ins, msg.index);
	}
}
