#ifndef APP_SUBSCRIBERS_HPP_
#define APP_SUBSCRIBERS_HPP_

#include <app/uavcan.hpp>

// Message type includes

bool is_node_operational = false;

// Subscriber objects - keep static, only access through these functions
static uavcan::Subscriber<uavcan::protocol::NodeStatus>* NodeStatus_sub;

void NodeStatus_sub_callback(const uavcan::ReceivedDataStructure<uavcan::protocol::NodeStatus>& msg) {
	if (msg.getSrcNodeID() == 10) {
		if (msg.health == msg.HEALTH_OK) {
			is_node_operational = true;
		} else {
			is_node_operational = false;
		}
	}
}

/**@brief Function to initialize UAVCAN subscribers.
 *
 */
uint8_t subscribers_init() {
	uint8_t rc = 0;

	auto* node = get_node();

	NodeStatus_sub = new uavcan::Subscriber<uavcan::protocol::NodeStatus>(*node);

	NodeStatus_sub->start(NodeStatus_sub_callback);


	return rc;
}


#endif
