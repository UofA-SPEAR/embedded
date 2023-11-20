import can
import struct
import time
import math

with can.Bus(interface='socketcan', channel='can0', receive_own_messages=True) as bus:
    arbitration_id = (1 << 24) | (0x11 << 8) | 0x1
    velocity = math.pi/16
    v_dir = -math.pi/16
    while (True):
        velocity += v_dir
        data = struct.pack("Bf", 0x2, velocity)
        print(velocity)
        if (velocity <= -math.pi or velocity >= math.pi):
            v_dir *= -1.0

        message = can.Message(arbitration_id=arbitration_id,
                              is_extended_id=True,
                              data=data)
        bus.send(message, timeout=0.2)

        time.sleep(1)
