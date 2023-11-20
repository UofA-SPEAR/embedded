#!/usr/bin/env python
import can
import struct
import sys
import math

with can.Bus(interface='socketcan', channel='can0',
             receive_own_messages=True) as bus:
    arbitration_id = (1 << 24) | (0x11 << 8) | 0x1
    if (len(sys.argv) < 2):
        print("Missing argument")
        sys.exit()
    angle = int(sys.argv[2]) / 180 * math.pi
    data = struct.pack("Bf", int(sys.argv[1]), angle)
    message = can.Message(arbitration_id=arbitration_id,
                          is_extended_id=True,
                          data=data)
    bus.send(message, timeout=0.2)
