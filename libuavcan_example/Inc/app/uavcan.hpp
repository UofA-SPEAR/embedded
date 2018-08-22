#ifndef APP_UAVCAN_HPP_
#define APP_UAVCAN_HPP_

#include <uavcan_stm32/uavcan_stm32.hpp>

// UAVCAN specific settings
#define NODE_MEM_POOL_SIZE 16384

typedef uavcan::Node<NODE_MEM_POOL_SIZE> Node;
uavcan::Node<NODE_MEM_POOL_SIZE>* node;

static uavcan_stm32::CanInitHelper<254> can;

/**@brief Function that returns a reference to a consistent UAVCAN node object.
 *
 * @retval node reference to node object
 */
Node* get_node() {
	return node;
}

/**@brief Function that initializes UAVCAN node
 *
 */
uint8_t uavcan_init(uint8_t node_id,
		char* node_name,
		uavcan::protocol::HardwareVersion hw_ver,
		uavcan::protocol::SoftwareVersion sw_ver) {

	node = new uavcan::Node<NODE_MEM_POOL_SIZE>(can.driver, uavcan_stm32::SystemClock::instance());

	node->setNodeID(node_id);
	node->setName(node_name);
	node->setSoftwareVersion(sw_ver);
	node->setHardwareVersion(hw_ver);

	return node->start();
}

/**@brief Function to set UAVCAN mode operational
 *
 * @note Must be called after all other uavcan initializations
 */
void uavcan_setOperational() {
	node->setModeOperational();
}

/**@brief Function to spin uavcan node (in milliseconds)
 *
 * @param ms Time to spin in milliseconds
 */
void uavcan_spin_ms(uint64_t ms) {
	node->spin(uavcan::MonotonicDuration::fromMSec(ms));
}

#endif
