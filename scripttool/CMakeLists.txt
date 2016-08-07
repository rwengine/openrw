SET(SCRIPTTOOL scripttool)
add_executable(${SCRIPTTOOL} main.cpp)

target_link_libraries(${SCRIPTTOOL} 
rwengine
${OPENGL_LIBRARIES}
${BULLET_LIBRARIES}
${SDL2_LIBRARY})

install(TARGETS ${SCRIPTTOOL} RUNTIME DESTINATION "${BIN_DIR}")
