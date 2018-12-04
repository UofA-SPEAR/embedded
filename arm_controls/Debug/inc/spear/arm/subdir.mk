################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../inc/spear/arm/arm_JointCommand.c \
../inc/spear/arm/arm_JointStatus.c 

OBJS += \
./inc/spear/arm/arm_JointCommand.o \
./inc/spear/arm/arm_JointStatus.o 

C_DEPS += \
./inc/spear/arm/arm_JointCommand.d \
./inc/spear/arm/arm_JointStatus.d 


# Each subdirectory must supply rules for building sources it contributes
inc/spear/arm/%.o: ../inc/spear/arm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103RBTx -DNUCLEO_F103RB -DDEBUG -DSTM32F103xB -DUSE_HAL_DRIVER -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc/Legacy" -I"/home/jacob/Ac6/workspace/arm_controls/inc/canard" -I"/media/jacob/docs/Coding/spear/embedded/common/uavcan_dsdl/libcanard_dsdlc_generated" -I"/home/jacob/Ac6/workspace/arm_controls/Utilities/STM32F1xx_Nucleo" -I"/home/jacob/Ac6/workspace/arm_controls/inc" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/device" -I"/home/jacob/Ac6/workspace/arm_controls/CMSIS/core" -I"/home/jacob/Ac6/workspace/arm_controls/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

