import can
import struct
import time
import math

step_size = 0.25*math.pi
with can.Bus(interface='socketcan', channel='can0',
             receive_own_messages=True) as bus:
    arbitration_id = (1 << 24) | (0x11 << 8) | 0x1
    position = step_size
    dir_step = -step_size

    while (True):
        position += dir_step
        data = struct.pack("Bf", 0x2, position)
        print(position)
        # Go the other way
        if (position <= -step_size or position >= step_size):
            dir_step *= -1.0

        message = can.Message(arbitration_id=arbitration_id,
                              is_extended_id=True,
                              data=data)
        bus.send(message, timeout=0.2)

        time.sleep(2)
