#ifndef __COMS_HPP
#define __COMS_HPP
#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/uavcan.hpp>
#include <etl/string.h>

constexpr unsigned NodePoolSize = 8192;
uavcan::Node<NodePoolSize>& getNode();
void comsInit(etl::string<40> const& deviceName, uint8_t deviceId, uint32_t bitrate);
#endif