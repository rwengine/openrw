add_executable(viewer main.cpp ../framework/LoaderIPL.cpp ../framework/LoaderIMG.cpp ../framework/LoaderDFF.cpp ../framework/TextureLoader.cpp)

target_link_libraries( viewer sfml-graphics sfml-window sfml-system GL GLEW )

install(TARGETS viewer RUNTIME DESTINATION bin)
