# fixme: conan support
find_package(Qt5 REQUIRED COMPONENTS Gui)
add_executable(rwfontmap
    rwfontmap.cpp
    )

target_link_libraries(rwfontmap
    PUBLIC
        rwcore
        rwdep::freetype
        rwdep::boost_program_options
        Qt5::Gui
    )
