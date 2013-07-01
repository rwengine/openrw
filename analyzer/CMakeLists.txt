add_executable(analyzer main.cpp)

include_directories(../framework2/include)

target_link_libraries( analyzer renderware sfml-graphics )

install(TARGETS analyzer RUNTIME DESTINATION bin)
