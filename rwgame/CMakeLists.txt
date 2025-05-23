configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

add_library(librwgame STATIC
    GitSHA1.h
    "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp"

    RWConfig.inc
    RWConfig.hpp
    RWConfig.cpp

    GameBase.hpp
    GameBase.cpp
    RWGame.hpp
    RWGame.cpp
    GameWindow.hpp
    GameWindow.cpp

    RWImGui.cpp
    RWImGui.hpp

    HUDDrawer.hpp
    HUDDrawer.cpp
    MenuSystem.hpp
    MenuSystem.cpp
    GameInput.hpp
    GameInput.cpp

    game.hpp
    WindowIcon.hpp

    StateManager.hpp
    StateManager.cpp
    State.hpp
    State.cpp

    states/LoadingState.hpp
    states/LoadingState.cpp
    states/IngameState.hpp
    states/IngameState.cpp
    states/PauseState.hpp
    states/PauseState.cpp
    states/MenuState.hpp
    states/MenuState.cpp
    states/DebugState.hpp
    states/DebugState.cpp
    states/BenchmarkState.hpp
    states/BenchmarkState.cpp
    )

openrw_target_apply_options(
    TARGET librwgame
    CORE
    COVERAGE
    )

set_target_properties(librwgame
    PROPERTIES
        OUTPUT_NAME "rwgame"
        PREFIX "lib"
    )

target_include_directories(librwgame
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )
target_link_libraries(librwgame
    PUBLIC
        openrw::interface
        rwengine
        Boost::program_options
        SDL2::SDL2
    )

target_compile_definitions(librwgame
    PUBLIC
        RW_IMGUI
    )

target_link_libraries(librwgame
    PUBLIC
        imgui::sdl_gl3
    )

add_executable(rwgame
    main.cpp
    )


if(APPLE)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(BZ2_LIBRARY bz2 REQUIRED)
    find_library(COREHAPTICS_FRAMEWORK CoreHaptics)
    find_library(COREMEDIA_FRAMEWORK CoreMedia)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(VIDEOTOOLBOX_FRAMEWORK VideoToolbox)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(SECURITY_FRAMEWORK Security)

    target_link_libraries(rwgame
        PRIVATE librwgame
        ${AUDIOTOOLBOX_FRAMEWORK}
        ${COREAUDIO_FRAMEWORK}
        ${BZ2_LIBRARY}
        ${COREHAPTICS_FRAMEWORK}
        ${COREMEDIA_FRAMEWORK}
        ${AVFOUNDATION_FRAMEWORK}
        ${VIDEOTOOLBOX_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${SECURITY_FRAMEWORK}
        z
        iconv
        lzma
    )
else()
    target_link_libraries(rwgame
        PRIVATE
        librwgame
    )

endif()

openrw_target_apply_options(
    TARGET rwgame
    CORE
    COVERAGE
    INSTALL INSTALL_PDB
    )
