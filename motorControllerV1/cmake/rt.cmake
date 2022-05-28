include(cmake/checkMCU.cmake)
add_subdirectory(cmake/rt)
target_link_libraries(${EXECUTABLE} PRIVATE chibiosrt)