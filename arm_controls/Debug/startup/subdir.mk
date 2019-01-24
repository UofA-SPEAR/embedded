################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f103xb.s 

OBJS += \
./startup/startup_stm32f103xb.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	arm-none-eabi-as -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc/Legacy" -I"/home/jacob/Ac6/workspace/arm_controls/Utilities/STM32F1xx_Nucleo" -I"/home/jacob/Ac6/workspace/arm_controls/inc" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/device" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/core" -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


