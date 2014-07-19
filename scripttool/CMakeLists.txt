SET(SCRIPTTOOL scripttool)
add_executable(${SCRIPTTOOL} main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/rwengine/include)

install(TARGETS ${SCRIPTTOOL} RUNTIME DESTINATION bin)
