#ifndef APP_PUBLISHERS_HPP_
#define APP_PUBLISHERS_HPP_

// This should not be hardcoded here, maybe do auto id assignment later

#include <app/uavcan.hpp>

// Message type includes
#include <uavcan/protocol/NodeStatus.hpp>

// Publisher objects - keep static, only access these through functions
static uavcan::Publisher<uavcan::protocol::NodeStatus>* status_pub;


/**@brief Function to initialize publishers
 *
 */
uint8_t publishers_init() {
    uint8_t rc = 0;

    auto* node = get_node();

    status_pub = new uavcan::Publisher<uavcan::protocol::NodeStatus>(*node);

    rc = status_pub->init();
    if (rc < 0) {
        return rc;
    }

    return rc;
}

/**@brief Function to publish to NodeStatus
 *
 */
int publish_NodeStatus(uint8_t health, uint8_t mode, uint16_t status_code) {
    uavcan::protocol::NodeStatus status_msg;

    // Build message
    status_msg.health = health;
    status_msg.mode = mode;
    status_msg.vendor_specific_status_code = status_code;

    // Publish message
    return status_pub->broadcast(status_msg);
}


#endif
