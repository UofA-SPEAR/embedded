#ifndef COMS_H_
#define COMS_H_
#include "canard.h"

void coms_init(void);
void updateComs(void);


// Should this go here or in another file?
// Paramater indexes to use
typedef enum {
  PARAMETER_motor1_actuator_id,
} parameter_id_t;

extern CanardInstance m_canard_instance;

extern uint64_t can_timestamp_usec;

extern uint8_t inout_transfer_id;

// Public variables to set nodestatus
extern uint32_t node_health;
extern uint32_t node_mode;

void publish_NodeStatus(void);

#endif
