struct ArgumentsFromApplication
{
    bool linger;        ///< Whether to boot immediately or to wait for commands.

    std::uint16_t uavcan_serial_node_id;                    ///< Invalid if unknown.

    std::uint32_t uavcan_can_bitrate_first;             ///< Zeros if unknown.
    std::uint32_t uavcan_can_bitrate_second;
    std::uint8_t                            uavcan_can_protocol_version;    ///< v0 or v1; 0xFF if unknown.
    std::uint8_t                            uavcan_can_node_id;             ///< Invalid if unknown.

    std::uint8_t                  trigger_node_index;       ///< 0 - serial, 1 - CAN, >1 - none.
    std::uint16_t                 file_server_node_id;      ///< Invalid if unknown.
    std::array<std::uint8_t, 256> remote_file_path;         ///< Null-terminated string.
};