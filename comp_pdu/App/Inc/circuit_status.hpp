#ifndef COMP_PDU_STATUS_HPP
#define COMP_PDU_STATUS_HPP

#include <uavcan/equipment/power/CircuitStatus.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>
#include "coms.hpp"

typedef uavcan::Publisher<uavcan::equipment::power::CircuitStatus> CircuitStatusPublisher;
typedef uavcan::Node<NodePoolSize> Node;

CircuitStatusPublisher& getCircuitPub();
void update_circuit_status(float voltage, float current);

#endif
