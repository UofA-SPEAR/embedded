set(LINKER_SRC ${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/ld)
target_compile_options(${EXECUTABLE} PRIVATE
${UDEFS}
)
set(libPath C:/ARMGCC/10_2021.07/arm-none-eabi/lib/thumb/v7e-m+fp/hard
C:/ARMGCC/10_2021.07/lib/gcc/arm-none-eabi/10.3.1/thumb/v7e-m+fp/hard)

target_link_directories(${EXECUTABLE} PRIVATE 
${libPath})
    
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
target_link_libraries(${EXECUTABLE} PUBLIC ${ULIBS})

add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)