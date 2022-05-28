#include "circuit_status.hpp"

#define CIRCUIT_ID 1

typedef uavcan::Publisher<uavcan::equipment::power::CircuitStatus> CircuitPublisher;
typedef uavcan::Node<NodePoolSize> Node;

CircuitStatusPublisher& getCircuitPub() {
	static CircuitStatusPublisher comp_pdu_pub(getNode());
	return comp_pdu_pub;
}

void update_circuit_status(float voltage, float current) {
	auto& publisher = getCircuitPub();
	uavcan::equipment::power::CircuitStatus comp_pdu_status;
	comp_pdu_status.circuit_id = CIRCUIT_ID;
	comp_pdu_status.voltage = voltage;
	comp_pdu_status.current = current;
	comp_pdu_status.error_flags = 0;
	publisher.broadcast(comp_pdu_status);
}
