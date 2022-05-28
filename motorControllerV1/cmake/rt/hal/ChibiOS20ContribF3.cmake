FILE(GLOB CHIBIOS_CONTRIB_SRC
${CHIBIOS_CONTRIB}/os/hal/src/*.c
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/*.c
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/*.c
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1/*.c
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/OPAMPv1/*.c)

target_include_directories(${chibiOS} PUBLIC
${CHIBIOS_CONTRIB}/os/hal/include
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1
${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/OPAMPv1
)

target_sources(${chibiOS} PRIVATE
${CHIBIOS_CONTRIB_SRC}
)