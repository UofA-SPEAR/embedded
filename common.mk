######
# Makefile based on STM32Cube auto-generated makefile
#
# TODO:
# - clean up, maybe optimize builds more
# - add windows support
# - add *proper* retargeting (this is mostly restructuring some files and adding make variables)
#
# Mediocre stuff about how I'm doing this:
# - First build times are loooooong. make -j4 will help, but we're still compiling lots we do't need.
#     This is mostly because I don't want people to have to specifiy which source files they need.

####################
# Common directories
#
# These variables need to be defined in your makefile:
# 	COMMON_DIR - the common/ folder, where everything not for a specific project is held
# 	PROJ_DIR - Location of the current project
# 	MCU_SERIES - f0, f3, etc... the series for STM32 you are using
# 	MCU_PARTNO - Full partno you are using, in the HAL style, e.g. STM32F303xC
#
# 	SOURCE_DIRS - project specific sources, can be empty
# 	LDSCRIPT - The locaton of your linker script.
#
#
# 	CPU, see example makefile
# 	FPU, see example makefile
# 	FLOAT-ABI - see example makefile
#

# Source and header files are in same directory
LIBCANARD_DIR := $(COMMON_DIR)/libcanard
LIBCANARD_STM32_DIR := $(LIBCANARD_DIR)/drivers/stm32

CANARD_DSDL_COMPILED_DIR := $(COMMON_DIR)/uavcan_dsdl/libcanard_dsdlc_generated

HAL_DRIVER_DIR := $(COMMON_DIR)/HAL_Drivers/$(MCU_SERIES)
HAL_DRIVER_INC_DIR := $(HAL_DRIVER_DIR)/Inc
HAL_DRIVER_SRC_DIR := $(HAL_DRIVER_DIR)/Src

CMSIS_DIR := $(COMMON_DIR)/CMSIS
CMSIS_CORE_DIR := $(CMSIS_DIR)/core
CMSIS_DEVICE_DIR := $(CMSIS_DIR)/device


SOURCE_DIRS += \
		$(LIBCANARD_DIR) \
		$(LIBCANARD_STM32_DIR) \
		$(HAL_DRIVER_SRC_DIR) \
		$(PROJ_DIR)/src \


# Find C sources:
C_SOURCES := $(shell find $(SOURCE_DIRS) -maxdepth 1 ! -name "*template.c" -name "*.c")
# DSDL compiled sources need to be treated differently
C_SOURCES += $(shell find $(CANARD_DSDL_COMPILED_DIR) -name "*.c")

# Find C++ sources
CPP_SOURCES := $(shell find $(SOURCE_DIRS) -maxdepth 1 -name "*.cpp")

# Startup file, must be changed for different processors
# TODO: figure out how to make these depend on the processor you define
ASM_SOURCES := $(PROJ_DIR)/startup/startup_stm32.s


##############
# Build tools
##############
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CXX = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)


# Definitions to be called when compiling
AS_DEFS :=

# Common to C and C++
C_DEFS := \
	-DUSE_HAL_DRIVER \
	-D$(MCU_PARTNO) \


# Includes for compiling
AS_INCLUDES := 

# Common to C and C++
C_INCLUDES := \
	-I$(PROJ_DIR)/inc \
	-I$(HAL_DRIVER_INC_DIR) \
	-I$(CMSIS_CORE_DIR) \
	-I$(CMSIS_DEVICE_DIR) \
	-I$(LIBCANARD_DIR) \
	-I$(LIBCANARD_STM32_DIR) \
	-I$(CANARD_DSDL_COMPILED_DIR) \


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -std=gnu99
CXXFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
CXXFLAGS += -g -gdwarf-2
endif


# Generate dependency information
# As far as I understand, this makes it so you can recompile correctly when header files change
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CXXFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


##################
# Linker stuff!
##################

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

ifdef ARM_MATH
	LIBS += -larm_math
	LIBDIR += -L$(CMSIS_DIR)/lib
endif



#################
# Building!
#################
BUILD_DIR := $(PROJ_DIR)/build

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# C++ objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		


#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)


####################
# Flashing/Debugging
####################

flash: $(BUILD_DIR)/$(TARGET).elf
	openocd -f $(COMMON_DIR)/$(MCU_SERIES)_openocd.cfg \
		-c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit" \

debug: $(BUILD_DIR)/$(TARGET).elf
	openocd -f $(COMMON_DIR)/$(MCU_SERIES)_openocd.cfg
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
