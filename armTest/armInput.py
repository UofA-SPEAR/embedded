#!/usr/bin/env python
from inputs import get_gamepad
from threading import Thread
import struct
import can
import queue

commands = queue.Queue()


def can_thread():
    with can.Bus(interface='socketcan', channel='can0',
                 receive_own_messages=True) as bus:
        arbitration_id = (1 << 24) | (0x10 << 8) | 0x1
        while (True):
            actuator_id, velocity = commands.get()
            print(actuator_id, velocity)
            data = struct.pack("Bf", actuator_id, velocity)
            message = can.Message(arbitration_id=arbitration_id,
                                  is_extended_id=True,
                                  data=data)
            bus.send(message, timeout=0.2)
            pass


def handle_absolute(event):
    match event.code:
        # Left stick
        case "ABS_X":  # Left stick left right
            if (abs(event.state) < 15000):
                velocity = 0
            else:
                velocity = -event.state / 32768
            commands.put((35, velocity))
        case "ABS_Y":  # Left stick up down
            if (abs(event.state) < 15000):
                velocity = 0
            else:
                velocity = -event.state / 32768
            commands.put((34, velocity))
        case "ABS_RX":  # Right stick left right
            if (abs(event.state) < 15000):
                velocity = 0
            else:
                velocity = event.state / 32768
            commands.put((30, velocity))
        case "ABS_RY":  # Right stick up down
            if (abs(event.state) < 15000):
                velocity = 0
            else:
                velocity = -event.state / 32768
            commands.put((31, velocity))
        # D-pad
        case "ABS_HAT0X":
            pass
            # commands.put((33, -event.state))
        case "ABS_HAT0Y":
            commands.put((32, event.state))
        case "ABS_Z":  # Left trigger
            pass
        case "ABS_RZ":  # Right trigger
            pass


def handle_key(event):
    match event.code:
        case "BTN_TL":  # Left bumper
            pass
        case "BTN_TR":  # Right bumper
            pass
        case "BTN_NORTH":  # X Button
            pass
        case "BTN_SOUTH":  # A Button
            commands.put((23, -event.state))
        case "BTN_EAST":  # B Button
            commands.put((23, event.state))
        case "BTN_WEST":  # Y button
            pass


def main():
    can_handler = Thread(target=can_thread)
    can_handler.start()
    while True:
        events = get_gamepad()
        for event in events:
            # print(event.ev_type, event.code, event.state)
            match event.ev_type:
                case "Absolute":
                    handle_absolute(event)
                case "Key":
                    handle_key(event)


if __name__ == "__main__":
    main()
