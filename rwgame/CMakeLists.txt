find_package(Boost COMPONENTS program_options REQUIRED)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

add_executable(rwgame
  GitSHA1.h
  "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp"

  main.cpp

  GameBase.hpp
  GameBase.cpp
  RWGame.cpp

  GameConfig.cpp
  GameWindow.cpp

  StateManager.hpp
  StateManager.cpp
  State.cpp

  MenuSystem.hpp
  MenuSystem.cpp

  GameInput.hpp
  GameInput.cpp

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

  DrawUI.cpp
  )

target_include_directories(rwgame
  PRIVATE
  "${CMAKE_SOURCE_DIR}/rwgame"
  )

target_link_libraries(rwgame
  PRIVATE
  rwengine
  ${SDL2_LIBRARY}
  ${Boost_LIBRARIES}
  )

install(TARGETS rwgame RUNTIME DESTINATION "${BIN_DIR}")
