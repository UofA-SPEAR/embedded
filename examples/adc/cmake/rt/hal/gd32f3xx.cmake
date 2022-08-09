set(PORT_FILE ChibiOS20PORTv7.cmake)
include(hal/ChibiOS20PORTv7.cmake)
target_include_directories(${chibiOS} PUBLIC
${CHIBIOS}/os/hal/ports/common/ARMCMx
${CHIBIOS}/os/hal/ports/STM32/STM32F1xx
${CHIBIOS}/os/hal/ports/STM32/LLD/CANv1
${CHIBIOS}/os/hal/ports/STM32/LLD/DACv1
${CHIBIOS}/os/hal/ports/STM32/LLD/DMAv1
${CHIBIOS}/os/hal/ports/STM32/LLD/GPIOv1
${CHIBIOS}/os/hal/ports/STM32/LLD/I2Cv1
${CHIBIOS}/os/hal/ports/STM32/LLD/RTCv1
${CHIBIOS}/os/hal/ports/STM32/LLD/SDIOv1
${CHIBIOS}/os/hal/ports/STM32/LLD/SPIv1
${CHIBIOS}/os/hal/ports/STM32/LLD/SYSTICKv1
${CHIBIOS}/os/hal/ports/STM32/LLD/TIMv1
${CHIBIOS}/os/hal/ports/STM32/LLD/USARTv1
${CHIBIOS}/os/hal/ports/STM32/LLD/USBv1
${CHIBIOS}/os/hal/ports/STM32/LLD/xWDGv1
${CHIBIOS}/os/common/portability/GCC

${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC
${CHIBIOS}/os/common/startup/ARMCMx/devices/GD32F3xx
${CHIBIOS}/os/common/ext/ARM/CMSIS/Core/Include
${CHIBIOS}/os/common/ext/ST/GD32F3xx

${CHIBIOS}/os/common/portability/GCC
${CHIBIOS}/os/common/ports/ARM-common
${CHIBIOS}/os/common/ports/ARMv7-M

${CHIBIOS_PORT_INC}
)

FILE(GLOB CHIBIOS_PORT
${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c
${CHIBIOS}/os/hal/ports/STM32/STM32F1xx/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/CANv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/DACv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/DMAv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/GPIOv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/I2Cv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/RTCv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/SDIOv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/SPIv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/SYSTICKv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/TIMv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/USARTv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/USBv1/*.c
${CHIBIOS}/os/hal/ports/STM32/LLD/xWDGv1/*.c

${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/crt0_v7m.S
${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/vectors.S
${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/crt1.c
)

target_compile_definitions(${chibiOS} PUBLIC
CORTEX_USE_FPU=${IS_FPU}
)

target_sources(${chibiOS} PRIVATE
${CHIBIOS_PORT}
${CHIBIOS_PORT_SRC}
)