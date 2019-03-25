######
# Makefile based on STM32Cube auto-generated makefile
#
# TODO:
# - Verify/fix C++ support
# - Get rid of wchar_t warning
# - Genericise, so move most of this into a single makefile that is included by each project
# - add support for migrating to a different STM32 target
# - add debugging/flashing (just OpenOCD stuff really)
# - clean up, maybe optimize builds more
# - add windows support
#
# Mediocre stuff about how I'm doing this:
# - First build times are loooooong. make -j4 will help, but we're still compiling lots we do't need.
#     This is mostly because I don't want people to have to specifiy which source files they need.


######################################
# target
######################################
TARGET = arm_controller


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

# These all have to be defined for your specific processor
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard

COMMON_DIR := ../common
PROJ_DIR := ./
MCU_SERIES := f3
MCU_PARTNO := STM32F303xC

include ../common.mk
