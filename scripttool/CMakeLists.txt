SET(SCRIPTTOOL scripttool)
add_executable(${SCRIPTTOOL} main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/rwengine/include)

target_link_libraries(${SCRIPTTOOL} 
rwengine
${OPENGL_LIBRARIES}
${BULLET_LIBRARIES}
${SDL2_LIBRARIES})

install(TARGETS ${SCRIPTTOOL} RUNTIME DESTINATION bin)
