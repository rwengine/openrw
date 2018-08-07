# fixme: conan support
find_package(Freetype REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Gui)
add_executable(rwfontmap
    rwfontmap.cpp
    )

target_link_libraries(rwfontmap
    PUBLIC
        rwlib
        Freetype::Freetype
        Qt5::Gui
        Boost::program_options
    )
