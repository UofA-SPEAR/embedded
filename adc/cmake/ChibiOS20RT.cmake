
target_include_directories(${EXECUTABLE} PRIVATE
${CHIBIOS}/os/license
${CHIBIOS}/os/oslib/include
${CHIBIOS}/os/hal/osal/rt-nil
${CHIBIOS}/os/rt/include
${CHIBIOS}/os/various/cpp_wrappers
)

FILE(GLOB CHIBIOS_RT_SRC
${CHIBIOS}/os/hal/osal/rt-nil/osal.c
${CHIBIOS}/os/rt/src/*.c
${CHIBIOS}/os/various/cpp_wrappers/*.c
${CHIBIOS}/os/various/syscalls.c
${CHIBIOS}/os/oslib/src/*.c
)

target_sources(${EXECUTABLE} PRIVATE
${CHIBIOS_RT_SRC}
)