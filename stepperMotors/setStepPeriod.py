#!/usr/bin/env python
import can
import struct
import sys

with can.Bus(interface='socketcan', channel='can0',
             receive_own_messages=True) as bus:
    arbitration_id = (1 << 24) | (0x40 << 8) | 0x1
    if (len(sys.argv) < 3):
        print("Missing argument")
        sys.exit()
    period_us = int(sys.argv[2])
    actuator = int(sys.argv[1])
    data = struct.pack("BBBI", actuator, 1, 0, period_us)
    message = can.Message(arbitration_id=arbitration_id,
                          is_extended_id=True,
                          data=data)
    bus.send(message, timeout=0.2)
