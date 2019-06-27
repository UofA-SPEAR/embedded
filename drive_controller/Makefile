

######################################
# target
######################################
TARGET = drive_controller


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# Errors on Warnings?
WARNERROR = 1
# optimization
OPT = -Og
# Include arm_math stuff?
# Comment out if you don't need this, it causes a warning
ARM_MATH = 1

# These all have to be defined for your specific processor
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard

COMMON_DIR := ../common
PROJ_DIR := ./
MCU_SERIES := f3
MCU_PARTNO := STM32F303xC

# Project specific source directories, can be empty
SOURCE_DIRS :=


# link script
LDSCRIPT = $(PROJ_DIR)/LinkerScript.ld


include ../common.mk
