set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
cmake_minimum_required(VERSION 3.15.0)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

enable_language(C ASM CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
set(PROCESSOR_STACK_SIZE 0x1200)

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
set(EXCEPTION_STACK_SIZE 0x400)

# Define C warning options here.
set(CWARN -Wall -Wextra -Wundef -Wstrict-prototypes)

# Define C++ warning options here.
set(CPPWARN -Wall -Wextra -Wundef)

# List all user C define here, like -D_DEBUG=1
set(UDEFS
-DARM_MATH_CM4
)

# List the user directory to look for the libraries here
set(ULIBDIR
${CMAKE_SOURCE_DIR}/lib)

# List all user libraries here
set(ULIBS
libarm_cortexM4lf_math.a)

set(EXECUTABLE ${PROJECT_NAME}.elf)

# ChibiOS source directory
set(CHIBIOS ${CMAKE_SOURCE_DIR}/../ChibiOS)
set(CHIBIOS_CONTRIB ${CMAKE_SOURCE_DIR}/../ChibiOS-Contrib)
set(COMMON ../common)
# Every MCU has its own linkerscript
set(LINKERSCRIPT STM32F303xC.ld)

# set glob for source file
FILE(GLOB APP_SRC
App/Src/*.c
App/Src/*.cpp
${CHIBIOS}/os/various/syscalls.c
)

add_executable(${EXECUTABLE} 
${APP_SRC}
${UAVCAN_SRC}
)

include_directories(
App/cfg
boardCfg
flashDriver
)
set(BOARD_SRC ${CMAKE_SOURCE_DIR}/boardCfg/board.c ${CMAKE_SOURCE_DIR}/flashDriver/hal_efl_lld.c)

target_include_directories(${EXECUTABLE} PRIVATE
App/Inc
)
# option(LIBUAVCAN_SUPPORT "Build with libuavcan" ON)
# if(LIBUAVCAN_SUPPORT)
#   set(UAVCAN_PLATFORM stm32 CACHE STRING "uavcan stm32 platform")
#   add_subdirectory(${UAVCAN}/uavcanDriver/stm32/libuavcan/driver)
#   add_subdirectory(${UAVCAN}/libuavcan-v0)
# endif()
set(Micro STM32F3)
include(cmake/rt.cmake)

add_subdirectory(lib/etl)
target_link_libraries(${EXECUTABLE} PRIVATE etl)
include(../common/libuavcan.cmake)


#################################################################################################################



# set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5)
# set(DSP ${ROOT}/CMSIS/DSP)
# list(APPEND CMAKE_MODULE_PATH ${DSP})
# include(${DSP}/Toolchain/GCC.cmake)
# add_subdirectory(${DSP}/Source bin_dsp)
# target_link_libraries(${EXECUTABLE} PRIVATE CMSISDSP)

include(cmake/link.cmake)

SET_TARGET_PROPERTIES(${EXECUTABLE} PROPERTIES LINKER_LANGUAGE CXX)
