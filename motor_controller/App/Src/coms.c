#include <stdbool.h>

#include "canard.h"
//#include "canard_stm32.h"
#include "can.h"
#include "coms.h"
#include "flash_settings.h"
#include "main.h"
#include "settings.h"

#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#endif
#include "arm_math.h"
#include "ch.h"
#include "hal.h"
//#include "canard_stm32.h"

#include "uavcan/equipment/actuator/ArrayCommand.h"
#include "uavcan/equipment/actuator/Command.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/RestartNode.h"
#include "uavcan/protocol/param/GetSet.h"

// Small enough to not be too bad, large enough to be useful
#define DYNAMIC_ARRAY_BUF_SIZE 1000
#define RX_FIFO_LEN 10

// Dynamic array buffer for decoding messages
uint8_t dynamic_array_buf[DYNAMIC_ARRAY_BUF_SIZE];
// The below is necessary, otherwise a pointer to dynamic_array_buf resolves to
// NULL
uint8_t *p_dynamic_array_buf = (uint8_t *)dynamic_array_buf;

uint32_t node_health;
uint32_t node_mode;

uint8_t out_buf[100];

uint8_t inout_transfer_id;

static uint8_t actuator_id;

static void handle_RestartNode(CanardInstance *ins, CanardRxTransfer *transfer);
static void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer);
static void handle_Actuator_ArrayCommand(CanardInstance *ins,
                                         CanardRxTransfer *transfer);
static void handle_Actuator_Command(CanardInstance *ins,
                                    CanardRxTransfer *transfer);

static void run_actuator_command(uavcan_equipment_actuator_Command *cmd) {
  if (cmd->actuator_id == actuator_id) {
    motor_set(cmd->command_value);
  }
}

can_msg_handler can_request_handlers[] = {
    CAN_MSG_HANDLER(UAVCAN_PROTOCOL_PARAM_GETSET_ID,
                    UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE, handle_GetSet),
    CAN_MSG_HANDLER(UAVCAN_PROTOCOL_GETNODEINFO_ID,
                    UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE, handle_GetNodeInfo),
    CAN_MSG_HANDLER(UAVCAN_PROTOCOL_RESTARTNODE_ID,
                    UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE, handle_RestartNode),
    CAN_MSG_HANDLER_END};

can_msg_handler can_broadcast_handlers[] = {
    CAN_MSG_HANDLER(UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID,
                    UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE,
                    handle_Actuator_ArrayCommand),
    // this one shouldn't actually really be sent
    CAN_MSG_HANDLER(2155, UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_SIGNATURE,
                    handle_Actuator_Command),
    CAN_MSG_HANDLER_END};

void coms_init(void) {
  actuator_id = get_setting_int("spear.motor.actuator_id");
  can_init(NULL, can_broadcast_handlers, can_request_handlers);
}

/** @brief Handles ActuatorCommand messages
 *
 * Updates position values according to stuff
 */
static void handle_Actuator_ArrayCommand(CanardInstance *ins,
                                         CanardRxTransfer *transfer) {
  uavcan_equipment_actuator_ArrayCommand msg;
  (void)ins;

  // Pull message data
  uavcan_equipment_actuator_ArrayCommand_decode(transfer, transfer->payload_len,
                                                &msg, &p_dynamic_array_buf);

  for (int i = 0; i < msg.commands.len; i++) {
    run_actuator_command(&msg.commands.data[i]);
  }
}

static void handle_Actuator_Command(CanardInstance *ins,
                                    CanardRxTransfer *transfer) {
  uavcan_equipment_actuator_Command cmd;
  (void)ins;

  uavcan_equipment_actuator_Command_decode(transfer, transfer->payload_len,
                                           &cmd, &p_dynamic_array_buf);

  run_actuator_command(&cmd);
}

static void handle_RestartNode(CanardInstance *ins,
                               CanardRxTransfer *transfer) {
  uavcan_protocol_RestartNodeResponse rsp;
  uavcan_protocol_RestartNodeRequest msg;
  uint8_t rsp_msg_buf[8];  // doesn't need to be this big
  int32_t len;
  (void)ins;

  uavcan_protocol_RestartNodeRequest_decode(transfer, transfer->payload_len,
                                            &msg, &p_dynamic_array_buf);

  if (msg.magic_number == UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAGIC_NUMBER) {
    if (RunMotor_thread) chThdTerminate(RunMotor_thread);
    if (Heartbeat_thread) chThdTerminate(Heartbeat_thread);

    rsp.ok = true;
    len = uavcan_protocol_RestartNodeResponse_encode(&rsp, rsp_msg_buf);
    canardRequestOrRespond(ins, transfer->source_node_id,
                           UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE,
                           UAVCAN_PROTOCOL_RESTARTNODE_ID, &inout_transfer_id,
                           0, CanardResponse, (const void *)rsp_msg_buf, len);

    can_request_restart(true);
  }
}

static void handle_GetNodeInfo(CanardInstance *ins,
                               CanardRxTransfer *transfer) {
  uavcan_protocol_GetNodeInfoResponse out_msg;
  char name_device[] = "Arm Controller";
  out_msg.name.len = strlen(name_device);
  out_msg.name.data = (uint8_t *)(name_device);
  out_msg.software_version.major = 0;
  out_msg.software_version.minor = 1;
  out_msg.hardware_version.major = 1;
  out_msg.hardware_version.minor = 0;
  out_msg.hardware_version.certificate_of_authenticity.len = 0;

  // TODO hook this into main status
  out_msg.status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
  out_msg.status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
  out_msg.status.sub_mode = 0;
  out_msg.status.vendor_specific_status_code = 0;
  out_msg.status.uptime_sec = TIME_I2S(chVTGetSystemTime());

  uint8_t len = uavcan_protocol_GetNodeInfoResponse_encode(&out_msg, out_buf);

  canardRequestOrRespond(ins, transfer->source_node_id,
                         UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
                         UAVCAN_PROTOCOL_GETNODEINFO_ID, &inout_transfer_id,
                         30,  // again with the priorities
                         CanardResponse, out_buf, len);
}

void publish_NodeStatus(void) {
  uavcan_protocol_NodeStatus msg;
  uint16_t len;

  msg.health = node_health;
  msg.mode = node_mode;
  msg.sub_mode = 0;
  msg.vendor_specific_status_code = 0;
  msg.uptime_sec = TIME_I2S(chVTGetSystemTime());

  len = uavcan_protocol_NodeStatus_encode(&msg, out_buf);

  canardBroadcast(&m_canard_instance, UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                  UAVCAN_PROTOCOL_NODESTATUS_ID, &inout_transfer_id, 0, out_buf,
                  len);
}
