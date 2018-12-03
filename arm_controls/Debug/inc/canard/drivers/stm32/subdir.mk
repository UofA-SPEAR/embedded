################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../inc/canard/drivers/stm32/canard_stm32.c 

OBJS += \
./inc/canard/drivers/stm32/canard_stm32.o 

C_DEPS += \
./inc/canard/drivers/stm32/canard_stm32.d 


# Each subdirectory must supply rules for building sources it contributes
inc/canard/drivers/stm32/%.o: ../inc/canard/drivers/stm32/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103RBTx -DNUCLEO_F103RB -DDEBUG -DSTM32F103xB -DUSE_HAL_DRIVER -I"/home/jacob/Ac6/workspace/Blinkv2/HAL_Driver/Inc/Legacy" -I"/home/jacob/Ac6/workspace/Blinkv2/inc/canard" -I"/home/jacob/Ac6/workspace/Blinkv2/inc/canard/drivers/stm32" -I"/media/jacob/docs/Coding/spear/embedded/common/uavcan_dsdl/libcanard_dsdlc_generated" -I"/home/jacob/Ac6/workspace/Blinkv2/Utilities/STM32F1xx_Nucleo" -I"/home/jacob/Ac6/workspace/Blinkv2/inc" -I"/home/jacob/Ac6/workspace/Blinkv2/CMSIS/device" -I"/home/jacob/Ac6/workspace/Blinkv2/CMSIS/core" -I"/home/jacob/Ac6/workspace/Blinkv2/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


