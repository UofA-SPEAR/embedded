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

static void parameter_return_empty(CanardInstance* ins);
static void parameter_respond_from_id(CanardInstance* ins
		uavcan_protocol_param_GetSetRequest* p_msg);
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
	} else if (msg.index > NUM_PARAMETERS) {
		return_empty_message(ins);
	} else {
		// No defined behaviour when setting parameter with ID
		parameter_respond_from_id(ins, &msg);
	}
}

static void parameter_return_empty(CanardInstance* ins) {
	// TODO implement this function
}

static void return_current_value(CanardInstance* ins,
		uavcan_protocol_param_GetSetRequest* p_msg) {

		// return current value
		uavcan_protocol_param_GetSetResponse response;
		uint8_t out_buf[100];
		uint16_t index;
		uint8_t motor_select = 0;

		// If motor B, select from second set of 14 parameters
		if (p_msg->index > 13) {
			index = p_msg->index - 14;
			motor_select = 1;
		}

		// This should probably be another function
		switch (index) {
		case (0):
			// Motor enabled?
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].enabled;
			break;
		case (1):
			// Motor actuator ID
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].actuator_id;
			break;
		case (2):
			// Motor reversed?
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].reversed;
			break;
		case (3):
			// Motor continuous?
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].continuous;
			break;
		case (4):
			// Motor Kp
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].pid.Kp;
			break;
		case (5):
			// Motor Ki
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].pid.Ki;
			break;
		case (6):
			// Motor Kd
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].pid.Kd;
			break;
		case (7):
			// Motor encoder type
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.type;
			break;
		case (8):
			// Motor encoder min value
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.min;
			break;
		case (9):
			// Motor encoder max value
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.max;
			break;
		case (10):
			// Motor encoder to_radians
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.to_radians;
			break;
		case (11):
			// Motor encoder to metres
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.to_metres;
			break;
		case (12):
			// Motor encoder min endstop enabled?
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.endstop_min;
			break;
		case (13):
			// Motor encoder max value
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
			response.value.integer_value = saved_settings.motor[motor_select].encoder.endstop_max;
			break;

		default:
			// Something fucked up
			while(1);
		}

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

static void parameter_respond_from_id(CanardInstance* ins,
		uavcan_protocol_param_GetSetRequest* p_msg) {
	// TODO respond to parameter requests without names
}

static void parameter_respond_from_name(CanardInstance* ins,
	uavcan_protocol_param_GetSetRequest* p_msg) {
	char name_buf[92]; // max length of name is 92 bytes

	// Copy name into temporary buffer
	memcpy(name_buf, p_msg->name.data, p_msg->name.len);

	if (strcmp(name_buf, "spear.arm.motor") == 0) { // ignore stuff not in the right namespace

		// Take off the already checked bit so we can save time.
		uint8_t len = strlen("spear.arm.motor");

		for (int i = 0; i < NUM_PARAMETERS; i++) {
			if (strcmp(name_buf + len, parameters[i] + len) == 0) {
				// Make sure that the index is the correct value to pass it on
				// If the index is not correct the next functions WILL fail
				p_msg->index = i;

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

