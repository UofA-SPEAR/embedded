set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
add_subdirectory(lib/etl)
target_link_libraries(${EXECUTABLE} PRIVATE etl)

set(LIBUAVCAN ${COMMON}/uavcan_dsdl/libuavcan)
target_compile_definitions(${EXECUTABLE} PRIVATE
  UAVCAN_STM32_CHIBIOS=1
  UAVCAN_STM32_NUM_IFACES=1
  UAVCAN_STM32_TIMER_NUMBER=6
)

target_include_directories(${EXECUTABLE} PRIVATE
  ${LIBUAVCAN}/libuavcan/include
  ${LIBUAVCAN}/libuavcan_drivers/stm32/driver/include
  ${COMMON}/uavcan_dsdl/libuavcan_dsdlc_generated/dsdlc_generated
  ${COMMON}/libuavcan_driver
  ${COMMON}/etl/include
)

FILE(GLOB LIBUAVCAN_SRC
  ${LIBUAVCAN}/libuavcan/src/*.cpp
  ${LIBUAVCAN}/libuavcan/src/driver/*.cpp
  ${LIBUAVCAN}/libuavcan/src/marshal/*.cpp
  ${LIBUAVCAN}/libuavcan/src/node/*.cpp
  ${LIBUAVCAN}/libuavcan/src/protocol/*.cpp
  ${LIBUAVCAN}/libuavcan/src/transport/*.cpp
  ${LIBUAVCAN}/libuavcan_drivers/stm32/driver/src/*.cpp
  ${COMMON}/libuavcan_driver/coms.cpp
  ${COMMON}/libuavcan_driver/paramServer.cpp
  
)

target_sources(${EXECUTABLE} PRIVATE
  ${LIBUAVCAN_SRC}
)
