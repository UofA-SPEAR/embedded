/*
 * settings.c
 *
 *  Created on: Feb 3, 2019
 *      Author: David Lenfesty
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

static void parameter_respond_from_id(CanardInstance* ins, uint16_t index);
static void parameter_respond_from_name(CanardInstance* ins,
		uavcan_protocol_param_GetSetRequest* p_msg);

/** @brief Reads and writes to settings.
 *
 */
void handle_getSet(CanardInstance* ins, CanardRxTransfer* transfer) {
	if (transfer->payload_len > 300) { //ignore messages that are too long
		return;
	}

	uavcan_protocol_param_GetSetRequest msg;

	uavcan_protocol_param_GetSetRequest_decode(transfer, transfer->payload_len,
			&msg, &p_dynamic_array_buf);


	if (msg.name.len > 0) {
		parameter_respond_from_name(ins, &msg);
	} else {
		// No defined behaviour when setting parameter with ID
		parameter_respond_from_id(ins, &msg);
	}
}

static void return_current_value(CanardInstance* ins,
		uavcan_protocol_param_GetSetRequest* p_msg) {

		// return current value
		uavcan_protocol_param_GetSetResponse response;
		uint8_t out_buf[100];

		uint16_t index = p_msg->index;

		response.value.integer_value = p_msg; // figure this shit out
		response.name.data = (uint8_t *) parameters[index];
		response.name.len = strlen(parameters[index]);

		uint8_t len = uavcan_protocol_param_GetSetResponse_encode(&response, out_buf);

		canardBroadcast(ins,
				UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
				UAVCAN_PROTOCOL_PARAM_GETSET_ID,
				&inout_transfer_id,
				30, // TODO verify this priority
				out_buf,
				len);

}

static void parameter_respond_from_id(CanardInstance* ins, uint16_t index) {
	// TODO respond to parameter requests without names
}

static void parameter_respond_from_name(CanardInstance* ins,
	uavcan_protocol_param_GetSetRequest* p_msg) {
	char name_buf[92]; // max length of name is 92 bytes

	// Copy name into temporary buffer
	memcpy(name_buf, p_msg->name.data, p_msg->name.len);

	if (strcmp(name_buf, "spear.arm") == 0) { // ignore stuff not in the right namespace

		for (int i = 0; i < NUM_PARAMETERS; i++) {
			if (strcmp(name_buf, parameters[i]) == 0) {
				if (p_msg->value.union_tag == UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
					return_current_value(ins, p_msg);
				} else {
					// set actuator id
					actuator_id = p_msg->value.integer_value;
					// TODO implement flash writing/reading
				}
			}
		}
	}
}

