#include "ch.h"
#include "hal.h"

#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/protocol/NodeStatus.hpp>
#include "coms.hpp"

#include "circuit_status.hpp"

#define NODE_ID 5
#define NODE_NAME "spear.comp_pdu"
#define NODE_BITRATE 50000

void *__dso_handle = 0;

int main(void) {
	chSysInit();
	halInit();
	sdStart(&SD2, NULL);
	
	comsInit(NODE_NAME, NODE_ID, NODE_BITRATE);
	auto& node = getNode();
	auto& circuitPub = getCircuitPub();
	circuitPub.init();

	while (true)
	{
		const int result = node.spinOnce();
		if (result < 0) {
		}
		update_circuit_status(1.0, 2.0);
		node.setHealthOk();
	}
}


