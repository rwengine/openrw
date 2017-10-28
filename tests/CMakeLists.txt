##############################################################################
#    Unit Tests
##############################################################################

find_package(Boost COMPONENTS filesystem unit_test_framework system REQUIRED)

set(TEST_SOURCES
    main.cpp
    test_Globals.cpp
    test_Animation.cpp
    test_Archive.cpp
    test_Buoyancy.cpp
    test_Character.cpp
    test_Chase.cpp
    test_Cutscene.cpp
    test_Config.cpp
    test_Data.cpp
    test_FileIndex.cpp
    test_GameData.cpp
    test_GameWorld.cpp
    test_Globals.hpp
    test_Items.cpp
    test_Input.cpp
    test_Lifetime.cpp
    test_Loaderdff.cpp
    test_LoaderIPL.cpp
    test_Logger.cpp
    test_Menu.cpp
    test_Object.cpp
    test_ObjectData.cpp
    test_Pickup.cpp
    test_Renderer.cpp
    test_RWBStream.cpp
    test_SaveGame.cpp
    test_ScriptMachine.cpp
    test_State.cpp
    test_Text.cpp
    test_TrafficDirector.cpp
    test_Vehicle.cpp
    test_VisualFX.cpp
    test_Weapon.cpp
    test_World.cpp
    test_ZoneData.cpp

    # Hack in rwgame sources until there's a per-target test suite
    "${CMAKE_SOURCE_DIR}/rwgame/GameConfig.cpp"
    "${CMAKE_SOURCE_DIR}/rwgame/GameWindow.cpp"
    "${CMAKE_SOURCE_DIR}/rwgame/GameInput.cpp"
    )

add_executable(run_tests ${TEST_SOURCES})

target_compile_definitions(run_tests
    PRIVATE
        "RW_TEST_WITH_DATA=$<NOT:$<BOOL:${TESTS_NODATA}>>"
        "BOOST_TEST_DYN_LINK"
    )

target_include_directories(run_tests
    SYSTEM
    PRIVATE
        ${Boost_INCLUDE_DIRS}
    )

target_include_directories(run_tests
    PRIVATE
        "${CMAKE_SOURCE_DIR}/tests"
        "${CMAKE_SOURCE_DIR}/rwgame"
    )

target_link_libraries(run_tests
    PRIVATE
        rwengine
        SDL2::SDL2
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
    )

add_test(NAME UnitTests
    COMMAND "$<TARGET_FILE:run_tests>"
    )
