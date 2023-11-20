import can
import struct
import time

with can.Bus(interface='socketcan', channel='can0', receive_own_messages=True) as bus:
    arbitration_id = (1 << 24) | (0x10 << 8) | 0x1
    velocity = 0.1
    while (True):
        data = struct.pack("Bf", 0x1, velocity)
        message = can.Message(arbitration_id=arbitration_id,
                              is_extended_id=True,
                              data=data)
        bus.send(message, timeout=0.2)

        data = struct.pack("Bf", 0x2, velocity)
        message = can.Message(arbitration_id=arbitration_id,
                              is_extended_id=True,
                              data=data)
        bus.send(message, timeout=0.2)
        time.sleep(1)
