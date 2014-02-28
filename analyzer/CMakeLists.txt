add_executable(analyzer main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/rwengine/include)

target_link_libraries( analyzer rwengine sfml-graphics )

install(TARGETS analyzer RUNTIME DESTINATION bin)
