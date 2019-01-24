################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../inc/canard/drivers/avr/avr-can-lib/src/at90can.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_buffer.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_disable_dyn_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_error_register.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_get_buf_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_get_dyn_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_get_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_send_buf_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_send_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_set_dyn_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/at90can_set_mode.c \
../inc/canard/drivers/avr/avr-can-lib/src/can_buffer.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_buffer.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_error_register.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_dyn_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_read_id.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_regdump.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_send_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_dyn_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_mode.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_sleep.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_static_filter.c \
../inc/canard/drivers/avr/avr-can-lib/src/mcp2515_write_id.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000_buffer.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000_error_register.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000_get_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000_send_message.c \
../inc/canard/drivers/avr/avr-can-lib/src/sja1000_set_mode.c \
../inc/canard/drivers/avr/avr-can-lib/src/spi.c 

OBJS += \
./inc/canard/drivers/avr/avr-can-lib/src/at90can.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_buffer.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_disable_dyn_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_error_register.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_buf_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_dyn_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_send_buf_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_send_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_set_dyn_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_set_mode.o \
./inc/canard/drivers/avr/avr-can-lib/src/can_buffer.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_buffer.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_error_register.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_dyn_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_read_id.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_regdump.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_send_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_dyn_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_mode.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_sleep.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_static_filter.o \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_write_id.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_buffer.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_error_register.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_get_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_send_message.o \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_set_mode.o \
./inc/canard/drivers/avr/avr-can-lib/src/spi.o 

C_DEPS += \
./inc/canard/drivers/avr/avr-can-lib/src/at90can.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_buffer.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_disable_dyn_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_error_register.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_buf_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_dyn_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_get_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_send_buf_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_send_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_set_dyn_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/at90can_set_mode.d \
./inc/canard/drivers/avr/avr-can-lib/src/can_buffer.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_buffer.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_error_register.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_dyn_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_get_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_read_id.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_regdump.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_send_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_dyn_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_set_mode.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_sleep.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_static_filter.d \
./inc/canard/drivers/avr/avr-can-lib/src/mcp2515_write_id.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_buffer.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_error_register.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_get_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_send_message.d \
./inc/canard/drivers/avr/avr-can-lib/src/sja1000_set_mode.d \
./inc/canard/drivers/avr/avr-can-lib/src/spi.d 


# Each subdirectory must supply rules for building sources it contributes
inc/canard/drivers/avr/avr-can-lib/src/%.o: ../inc/canard/drivers/avr/avr-can-lib/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103RBTx -DNUCLEO_F103RB -DDEBUG -DSTM32F103xB -DUSE_HAL_DRIVER -I"/home/jacob/Ac6/workspace/Blinkv2/HAL_Driver/Inc/Legacy" -I"/home/jacob/Ac6/workspace/Blinkv2/inc/canard" -I"/home/jacob/Ac6/workspace/Blinkv2/inc/canard/drivers/stm32" -I"/media/jacob/docs/Coding/spear/embedded/common/uavcan_dsdl/libcanard_dsdlc_generated" -I"/home/jacob/Ac6/workspace/Blinkv2/Utilities/STM32F1xx_Nucleo" -I"/home/jacob/Ac6/workspace/Blinkv2/inc" -I"/home/jacob/Ac6/workspace/Blinkv2/CMSIS/device" -I"/home/jacob/Ac6/workspace/Blinkv2/CMSIS/core" -I"/home/jacob/Ac6/workspace/Blinkv2/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


