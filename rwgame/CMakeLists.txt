configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

set(RWGAME_SOURCES
    GitSHA1.h
    "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp"

    main.cpp

    GameBase.hpp
    GameBase.cpp
    RWGame.hpp
    RWGame.cpp
    GameConfig.hpp
    GameConfig.cpp
    GameWindow.hpp
    GameWindow.cpp

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

add_executable(rwgame
    ${RWGAME_SOURCES}
    )

target_include_directories(rwgame
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

target_link_libraries(rwgame
    PRIVATE
        rwengine
        Boost::program_options
        SDL2::SDL2
    )

openrw_target_apply_options(
    TARGET rwgame
    COVERAGE
    INSTALL INSTALL_PDB
    )
