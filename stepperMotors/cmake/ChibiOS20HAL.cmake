FILE(GLOB CHIBIOS_HAL_SRC
${CHIBIOS}/os/hal/src/*.c
${CHIBIOS}/os/hal/lib/complex/mfs/*.c
${CHIBIOS}/os/hal/lib/streams/*.c
flashDriver/*.c
)

target_include_directories(${EXECUTABLE} PRIVATE
${CHIBIOS}/os/license
${CHIBIOS}/os/hal/include
${CHIBIOS}/os/hal/lib/streams
${CHIBIOS}/os/hal/lib/complex/mfs
flashDriver
)

target_sources(${EXECUTABLE} PRIVATE
${CHIBIOS_HAL_SRC}
)