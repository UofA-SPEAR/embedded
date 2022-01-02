cmake_minimum_required(VERSION 3.12.0)

project(pollingInput)

enable_language(C ASM CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


set(COMMON ../../common)
#################################################################################################################

# C specific options here (added to USE_OPT).
set(COPTS)

# C++ specific options here (added to USE_OPT).
set(CPPOPTS -fno-rtti)

# Enable this if you want link time optimizations (LTO).
set(USE_LTO FALSE)

# Enables the use of FPU (no, softfp, hard).
set(FPU hard)

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
set(PROCESSOR_STACK_SIZE 0x400)

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
set(EXCEPTION_STACK_SIZE 0x400)

# Define C warning options here.
set(CWARN -Wall -Wextra -Wundef -Wstrict-prototypes)

# Define C++ warning options here.
set(CPPWARN -Wall -Wextra -Wundef)

# List all user C define here, like -D_DEBUG=1
set(UDEFS )

# List the user directory to look for the libraries here
set(ULIBDIR )

# List all user libraries here
set(ULIBS )

set(EXECUTABLE ${PROJECT_NAME}.elf)

# ChibiOS source directory
set(CHIBIOS ${CMAKE_SOURCE_DIR}/../../ChibiOS)
set(CHIBIOS_CONTRIB ${CMAKE_SOURCE_DIR}/../ChibiOS-Contrib)

# Every MCU has its own linkerscript
set(LINKERSCRIPT STM32F303xC.ld)

FILE(GLOB APP_SRC
App/Src/*.c
App/Src/*.cpp
${COMMON}/SPEAR_SPEEDY_F3/*.c
)

add_executable(${EXECUTABLE} 
${APP_SRC}
)

target_include_directories(${EXECUTABLE} PRIVATE
App/cfg
App/Inc
${CHIBIOS}/os/hal/lib/streams
${COMMON}/SPEAR_SPEEDY_F3
${COMMON}/CMSIS/core
)

#################################################################################################################

include(cmake/ChibiOS20RT.cmake)
include(cmake/ChibiOS20HAL.cmake)
# include(cmake/ChibiOS20ContribF3.cmake)
include(cmake/stm32f3xx.cmake)
include(cmake/link.cmake)
