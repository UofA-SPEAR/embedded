
include(${PORT_FILE})

target_include_directories(${EXECUTABLE} PRIVATE
${CHIBIOS}/os/license
${CHIBIOS}/os/oslib/include
${CHIBIOS}/os/hal/osal/rt-nil
${CHIBIOS}/os/rt/include
${CHIBIOS_PORT_INC}
)

FILE(GLOB CHIBIOS_RT_SRC
${CHIBIOS}/os/hal/osal/rt-nil/osal.c
${CHIBIOS}/os/rt/src/*.c
${CHIBIOS}/os/oslib/src/*.c
${CHIBIOS}/os/various/syscalls.c
)

target_sources(${EXECUTABLE} PRIVATE
${CHIBIOS_RT_SRC}
${CHIBIOS_PORT_SRC}
)