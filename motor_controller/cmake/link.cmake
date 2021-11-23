
set(LINKER_SRC ${CHIBIOS}/os/common/startup/ARMCMx/compilers/GCC/ld)

target_compile_definitions(${EXECUTABLE} PRIVATE
${UDEFS})

target_compile_options(${EXECUTABLE} PRIVATE
${MCU}
-fdata-sections
-ffunction-sections
-fno-common
-fomit-frame-pointer 
-falign-functions=16
$<$<COMPILE_LANGUAGE:CXX>:${CPPWARN} ${CPPOPTS}> 
$<$<COMPILE_LANGUAGE:C>:${CWARN} ${COPTS}>
$<$<CONFIG:Debug>:-ggdb -O0>
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