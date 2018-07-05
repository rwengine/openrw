find_package(Qt5 REQUIRED COMPONENTS Gui)
add_executable(rwfontmap
    rwfontmap.cpp
    )

target_link_libraries(rwfontmap
    PUBLIC
        rwcore
        Freetype::Freetype
        Boost::program_options
        Qt5::Gui
    )

openrw_target_apply_options(
    TARGET rwfontmap
    FORMAT
    INSTALL INSTALL_PDB
    )
