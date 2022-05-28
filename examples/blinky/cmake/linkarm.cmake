set(LINKER_SRC ${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/ld)

target_compile_options(${EXECUTABLE} PRIVATE
# ${MCU} 
# --target=arm-arm-none-eabi
# -fdata-sections
# -ffunction-sections
# -fno-common
# -fomit-frame-pointer 
# -falign-functions=16
$<$<COMPILE_LANGUAGE:CXX>:
${MCU} 
--target=arm-arm-none-eabi
-fdata-sections
-ffunction-sections
-fno-common
-fomit-frame-pointer 
-falign-functions=16> 
$<$<COMPILE_LANGUAGE:C>:
${MCU} 
--target=arm-arm-none-eabi
-fdata-sections
-ffunction-sections
-fno-common
-fomit-frame-pointer 
-falign-functions=16> 
$<$<COMPILE_LANGUAGE:ASM>:${MCUASM}>

${UDEFS}
$<$<COMPILE_LANGUAGE:CXX>:${CPPWARN} ${CPPOPTS}> 
$<$<COMPILE_LANGUAGE:C>:${CWARN} ${COPTS}>
# $<$<CONFIG:Debug>:-ggdb -Og>
# $<$<CONFIG:Release>: -O2>
)

if(${USE_LTO} MATCHES TRUE)
target_compile_options(${EXECUTABLE} PRIVATE -flto)
endif()
# target_link_directories(${EXECUTABLE} PRIVATE ${LINKER_SRC})
target_link_options(${EXECUTABLE} PRIVATE
# ${MCULink}
# -nostartfiles
# -specs=nano.specs
# -lstdc++ 
# -lsupc++ 
# -lc 
# -lm 
# -lnosys
-T${LINKER_SRC}/${LINKERSCRIPT}
-Wl,--userlibpath=${LINKER_SRC}
-Wl,--map
-Wl,--script=${LINKERSCRIPT},--gc-sections
# -Wl,--script=${LINKERSCRIPT},--gc-sections
-Wl,--defsym=__process_stack_size__=${PROCESSOR_STACK_SIZE}
-Wl,--defsym=__main_stack_size__=${EXCEPTION_STACK_SIZE}
)

target_link_directories(${EXECUTABLE} PRIVATE ${ULIBDIR})
target_link_libraries(${EXECUTABLE} ${ULIBS})
SET_TARGET_PROPERTIES(${EXECUTABLE} PROPERTIES LINKER_LANGUAGE C)
set(CMAKE_CXX_LINK_EXECUTABLE
         "armclang  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE
         "armclang  <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
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