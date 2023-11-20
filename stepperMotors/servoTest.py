import can
import struct
import time
import math

step_size = 0.25*math.pi
with can.Bus(interface='socketcan', channel='can0', receive_own_messages=True) as bus:
    velocity = step_size
    v_dir = -step_size
    count = 0
    while (True):
        arbitration_id = (1 << 24) | (0x11 << 8) | 0x1
        print(velocity)
        if (count == 0):
            velocity += v_dir
            if (velocity <= -step_size or velocity >= step_size):
                v_dir *= -1.0
        count = (count + 1) % 100

        for i in range(6):
            data = struct.pack("Bf", 30+i, velocity)
            message = can.Message(arbitration_id=arbitration_id,
                                  is_extended_id=True,
                                  data=data)
            bus.send(message, timeout=0)
        arbitration_id = (1 << 24) | (0x10 << 8) | 0x1
        for i in range(6):
            data = struct.pack("Bf", 30+i, velocity)
            message = can.Message(arbitration_id=arbitration_id,
                                  is_extended_id=True,
                                  data=data)
            bus.send(message, timeout=0)

        time.sleep(0.005)
