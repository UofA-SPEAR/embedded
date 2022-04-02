#include "coms.hpp"
static mutex_t comsMtx;
uavcan_stm32::CanInitHelper<> can;
uavcan::Node<NodePoolSize>& getNode() {
    static uavcan::Node<NodePoolSize> node(can.driver, uavcan_stm32::SystemClock::instance());
    return node;
}

void comsInit(etl::string<40> const& deviceName, uint8_t deviceId, uint32_t bitrate)
{
    chMtxObjectInit(&comsMtx);
    int canerr = can.init(bitrate);
    chDbgAssert(canerr >= 0, "unable to init can bus peripheral");
    auto& node = getNode();
    node.setNodeID(deviceId);
    node.setName(deviceName.c_str());
    uavcan::protocol::SoftwareVersion sw_version;  // Standard type uavcan.protocol.SoftwareVersion
    sw_version.major = 1;
    node.setSoftwareVersion(sw_version);
    uavcan::protocol::HardwareVersion hw_version;  // Standard type uavcan.protocol.HardwareVersion
    hw_version.major = 1;
    node.setHardwareVersion(hw_version);
    int node_start_res = node.start();
    chDbgAssert(node_start_res >= 0, "unable to start libuavcan node");
    node.setModeOperational();
}