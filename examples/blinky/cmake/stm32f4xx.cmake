if(${FPU} MATCHES no)
set(IS_FPU FALSE)
set(FPU_CFG soft)
else()
set(IS_FPU TRUE)
set(FPU_CFG ${FPU})
endif()
set(MCU 
-mcpu=cortex-m4
-mthumb
-mfpu=fpv4-sp-d16
-mfloat-abi=${FPU_CFG}
)

target_include_directories(${EXECUTABLE} PRIVATE
${CHIBIOS}/os/hal/ports/common/ARMCMx
${CHIBIOS}/os/hal/ports/STM32/STM32F4xx
${CHIBIOS}/os/hal/ports/STM32/LLD/ADCv2
${CHIBIOS}/os/hal/ports/STM32/LLD/CANv1
${CHIBIOS}/os/hal/ports/STM32/LLD/CRYPv1
${CHIBIOS}/os/hal/ports/STM32/LLD/DACv1
${CHIBIOS}/os/hal/ports/STM32/LLD/DMAv2
${CHIBIOS}/os/hal/ports/STM32/LLD/EXTIv1
${CHIBIOS}/os/hal/ports/STM32/LLD/GPIOv2
${CHIBIOS}/os/hal/ports/STM32/LLD/I2Cv1
${CHIBIOS}/os/hal/ports/STM32/LLD/MACv1
${CHIBIOS}/os/hal/ports/STM32/LLD/OTGv1
${CHIBIOS}/os/hal/ports/STM32/LLD/QUADSPIv1
${CHIBIOS}/os/hal/ports/STM32/LLD/RTCv2
${CHIBIOS}/os/hal/ports/STM32/LLD/SPIv1
${CHIBIOS}/os/hal/ports/STM32/LLD/SDIOv1
${CHIBIOS}/os/hal/ports/STM32/LLD/TIMv1
${CHIBIOS}/os/hal/ports/STM32/LLD/USARTv1
${CHIBIOS}/os/hal/ports/STM32/LLD/xWDGv1
${CHIBIOS}/os/common/portability/GCC

${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC
${CHIBIOS}/os/common/startup/ARMCMx/devices/STM32F4xx
${CHIBIOS}/os/common/ext/ARM/CMSIS/Core/Include
${CHIBIOS}/os/common/ext/ST/STM32F4xx

${CHIBIOS}/os/common/portability/GCC
${CHIBIOS}/os/common/ports/ARM-common
${CHIBIOS}/os/common/ports/ARMv7-M
)

FILE(GLOB CHIBIOS_PORT
${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c
${CHIBIOS}/os/hal/ports/STM32/STM32F4xx/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/ADCv2/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/CANv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/CRYPv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/DACv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/DMAv2/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/EXTIv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/GPIOv2/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/I2Cv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/MACv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/OTGv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/QUADSPIv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/RTCv2/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/SPIv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/SDIOv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/TIMv1/*c
${CHIBIOS}/os/hal/ports/STM32/LLD/USARTv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/xWDGv1/*.c

${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/crt0_v7m.S
${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/vectors.S
${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/crt1.c
)

target_compile_definitions(${EXECUTABLE} PRIVATE
CORTEX_USE_FPU=${IS_FPU}
)

target_sources(${EXECUTABLE} PRIVATE
${CHIBIOS_PORT}
)