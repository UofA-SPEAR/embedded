set(LINKER_SRC ${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/ld)

target_compile_options(${EXECUTABLE} PRIVATE

# --target=arm-arm-none-eabi
$<$<COMPILE_LANGUAGE:CXX>:${MCU} --target=arm-arm-none-eabi> 
$<$<COMPILE_LANGUAGE:C>:${MCU} --target=arm-arm-none-eabi>
$<$<COMPILE_LANGUAGE:ASM>:${MCUASM}>
-fdata-sections
-ffunction-sections
-fno-common
-fomit-frame-pointer 
-falign-functions=16
${UDEFS}
$<$<COMPILE_LANGUAGE:CXX>:${CPPWARN} ${CPPOPTS}> 
$<$<COMPILE_LANGUAGE:C>:${CWARN} ${COPTS}>
$<$<CONFIG:Debug>:-ggdb -Og>
$<$<CONFIG:Release>: -O2>
)

if(${USE_LTO} MATCHES TRUE)
target_compile_options(${EXECUTABLE} PRIVATE -flto)
endif()

target_link_options(${EXECUTABLE} PRIVATE
${MCU}
-nostartfiles
-specs=nano.specs
-lstdc++ 
-lsupc++ 
-lc 
-lm 
-lnosys
-Wl,-Map=${PROJECT_NAME}.map,--cref
-Wl,--no-warn-mismatch
-Wl,--library-path=${LINKER_SRC}
-Wl,--script=${LINKERSCRIPT},--gc-sections
-Wl,--defsym=__process_stack_size__=${PROCESSOR_STACK_SIZE}
-Wl,--defsym=__main_stack_size__=${EXCEPTION_STACK_SIZE}
-Wl,--print-memory-usage
)


target_link_directories(${EXECUTABLE} PRIVATE ${ULIBDIR})
target_link_libraries(${EXECUTABLE} ${ULIBS})

# add_custom_target(
#     SPEEDY_TEMPLATE
#     COMMAND fmpp -C ${CMAKE_SOURCE_DIR}/boardCfg/cfg/board.fmpp
# )
# add_dependencies(${EXECUTABLE} SPEEDY_TEMPLATE)

# add_custom_command(TARGET ${EXECUTABLE}
#         POST_BUILD
#         COMMAND arm-none-eabi-size ${EXECUTABLE})

# Create hex file
# add_custom_command(TARGET ${EXECUTABLE}
#         POST_BUILD
#         COMMAND arm-none-eabi-objcopy -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
#         COMMAND arm-none-eabi-objcopy -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)