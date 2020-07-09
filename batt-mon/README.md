# Arm Controller Firmware #

Meant for running on arm controller boards.

Uses STM32F303. And a (possibly two in the future?) VNH5019 motor driver.

## You should know what you're doing ##

The various UAVCAN parameters that you can set ARE NOT LIMITED.

YOU CAN DO BAD THINGS TO THEM.

Don't set anything if you don't know what you're doing.

## TODO ##

- Change encoder stuff into a better method, and put in encoders.c
- Get rid of all errors/warnings
- figure out ADC things