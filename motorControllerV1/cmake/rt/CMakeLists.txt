cmake_minimum_required(VERSION 3.15.0)

project(rt)

set(chibiOS chibiosrt)

FILE(GLOB CHIBIOS_RT_SRC
${CHIBIOS}/os/hal/osal/rt-nil/osal.c
${CHIBIOS}/os/rt/src/*.c
${CHIBIOS}/os/various/cpp_wrappers/*.c
${CHIBIOS}/os/various/syscalls.c
${CHIBIOS}/os/oslib/src/*.c
)

add_library(${chibiOS} STATIC
${CHIBIOS_RT_SRC}
)

target_include_directories(${chibiOS} PUBLIC
${CHIBIOS}/os/license
${CHIBIOS}/os/oslib/include
${CHIBIOS}/os/rt/include
${CHIBIOS}/os/various/cpp_wrappers
)

FILE(GLOB CHIBIOS_HAL_SRC
${CHIBIOS}/os/hal/osal/rt-nil/osal.c
${CHIBIOS}/os/hal/src/*.c
${CHIBIOS}/os/hal/lib/complex/mfs/*.c
${CHIBIOS}/os/hal/lib/streams/*.c
${BOARD_SRC}
)
target_include_directories(${chibiOS} PUBLIC
${CHIBIOS}/os/hal/include
${CHIBIOS}/os/hal/osal/rt-nil
${CHIBIOS}/os/hal/lib/streams
${CHIBIOS}/os/hal/lib/complex/mfs
)

target_sources(${chibiOS} PRIVATE
${CHIBIOS_HAL_SRC}
)

if(${Micro} MATCHES STM32F3)
include(hal/stm32f3xx.cmake)
endif()

target_compile_options(${chibiOS} PUBLIC
${MCU}
-fdata-sections
-ffunction-sections
-fno-common
-fomit-frame-pointer 
-falign-functions=16
$<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wundef -fno-exceptions>
$<$<COMPILE_LANGUAGE:C>:-Wextra -Wundef -Wstrict-prototypes>
$<$<CONFIG:Debug>:-ggdb -Og>
$<$<CONFIG:Release>: -O2>
)
