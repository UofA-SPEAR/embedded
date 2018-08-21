#ifndef APP_PUBLISHERS_HPP_
#define APP_PUBLISHERS_HPP_

// This should not be hardcoded here, maybe do auto id assignment later
#define NODE_ID 30

#include <uavcan_stm32/uavcan_stm32.hpp>

// Message type includes
#include <uavcan/protocol/NodeStatus.hpp>

constexpr unsigned NodeMemoryPoolSize = 16384; // change this later?

uavcan_stm32::CanInitHelper<254> can;

/* Node object
 */
uavcan::Node<NodeMemoryPoolSize> node(can.driver, uavcan_stm32::SystemClock::instance());

/* Publisher objects
 * static because I want these to only be accessed via functions
 */
static uavcan::Publisher<uavcan::protocol::NodeStatus> status_pub(node);


/**@brief Function to start publisher
 */
int start_publisher() {
    int rc = 0;

    node.setNodeID(NODE_ID);
    node.setName("publisher example");

    rc = node.start();
    if (rc < 0) {
        return rc;
    }

    rc = status_pub.init();
    if (rc < 0) {
        return rc;
    }

    node.setModeOperational();

    return rc;
}

int publish_NodeStatus(uint8_t health, uint8_t mode, uint16_t status_code) {
    uavcan::protocol::NodeStatus status_msg;

    // Build message
    status_msg.health = health;
    status_msg.mode = mode;
    status_msg.vendor_specific_status_code = status_code;

    // Publish message
    return status_pub.broadcast(status_msg);
}


#endif
