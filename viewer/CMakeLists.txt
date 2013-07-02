add_executable(viewer main.cpp ../framework/LoaderDFF.cpp)

include_directories(../framework2/include)

target_link_libraries( viewer renderware sfml-graphics sfml-window sfml-system GL GLEW )

install(TARGETS viewer RUNTIME DESTINATION bin)
