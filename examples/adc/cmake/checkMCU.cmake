if(${FPU} MATCHES no)
set(IS_FPU FALSE)
set(FPU_CFG soft)
else()
set(IS_FPU TRUE)
set(FPU_CFG ${FPU})
endif()
if(${Micro} MATCHES STM32F3)
set(ARM_CPU cortex-m4)
set(MCU 
-mcpu=${ARM_CPU}
-mthumb
-mfpu=fpv4-sp-d16
-mfloat-abi=${FPU_CFG}
)
elseif(${Micro} MATCHES GD32F3)
set(ARM_CPU cortex-m4)
set(MCU 
-mcpu=${ARM_CPU}
-mthumb
-mfpu=fpv4-sp-d16
-mfloat-abi=${FPU_CFG}
)
endif()