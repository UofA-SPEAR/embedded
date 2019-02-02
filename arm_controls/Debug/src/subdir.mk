################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/clocks.c \
../src/coms.c \
../src/main.c \
../src/motor.c \
../src/pot.c \
../src/stm32f1xx_it.c \
../src/syscalls.c \
../src/system_stm32f1xx.c 

OBJS += \
./src/clocks.o \
./src/coms.o \
./src/main.o \
./src/motor.o \
./src/pot.o \
./src/stm32f1xx_it.o \
./src/syscalls.o \
./src/system_stm32f1xx.o 

C_DEPS += \
./src/clocks.d \
./src/coms.d \
./src/main.d \
./src/motor.d \
./src/pot.d \
./src/stm32f1xx_it.d \
./src/syscalls.d \
./src/system_stm32f1xx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103RBTx -DNUCLEO_F103RB -DDEBUG -DSTM32F103xB -DUSE_HAL_DRIVER -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc/Legacy" -I"/home/jacob/Ac6/workspace/arm_controls/inc/canard" -I"/media/jacob/docs/Coding/spear/embedded/common/uavcan_dsdl/libcanard_dsdlc_generated" -I"/home/jacob/Ac6/workspace/arm_controls/Utilities/STM32F1xx_Nucleo" -I"/home/jacob/Ac6/workspace/arm_controls/inc" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/device" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/core" -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


