include(cmake/checkMCU.cmake)
add_subdirectory(cmake/rt)
target_sources(${EXECUTABLE} PRIVATE ${CHIBIOS}/os/various/syscalls.c)
target_link_libraries(${EXECUTABLE} PUBLIC chibiosrt)