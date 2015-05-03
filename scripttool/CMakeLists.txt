SET(SCRIPTTOOL scripttool)
add_executable(${SCRIPTTOOL} main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/rwengine/include)

target_link_libraries(${SCRIPTTOOL} 
rwengine
sfml-graphics
sfml-window
sfml-system
${OPENGL_LIBRARIES}
GLEW
${BULLET_LIBRARIES})

install(TARGETS ${SCRIPTTOOL} RUNTIME DESTINATION bin)
