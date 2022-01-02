FILE(GLOB CHIBIOS_HAL_SRC
${CHIBIOS}/os/hal/src/*.c
)

target_include_directories(${EXECUTABLE} PRIVATE
${CHIBIOS}/os/license
${CHIBIOS}/os/hal/include
)

target_sources(${EXECUTABLE} PRIVATE
${CHIBIOS_HAL_SRC}
)