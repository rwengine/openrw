add_executable(datadump main.cpp ../framework/LoaderIMG.cpp)

target_link_libraries( datadump sfml-graphics sfml-window sfml-system GL GLEW )

install(TARGETS datadump RUNTIME DESTINATION bin)
