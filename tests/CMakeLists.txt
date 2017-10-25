##############################################################################
#    Unit Tests
##############################################################################

find_package(Boost COMPONENTS filesystem unit_test_framework REQUIRED)

set(TEST_SOURCES
    main.cpp
    test_globals.cpp
    test_animation.cpp
    test_archive.cpp
    test_buoyancy.cpp
    test_character.cpp
    test_chase.cpp
    test_cutscene.cpp
    test_config.cpp
    test_data.cpp
    test_FileIndex.cpp
    test_GameData.cpp
    test_GameWorld.cpp
    test_globals.hpp
    test_items.cpp
    test_Input.cpp
    test_lifetime.cpp
    test_loaderdff.cpp
    test_LoaderIPL.cpp
    test_Logger.cpp
    test_menu.cpp
    test_object.cpp
    test_object_data.cpp
    test_pickup.cpp
    test_renderer.cpp
    test_rwbstream.cpp
    test_SaveGame.cpp
    test_scriptmachine.cpp
    test_state.cpp
    test_text.cpp
    test_trafficdirector.cpp
    test_vehicle.cpp
    test_VisualFX.cpp
    test_weapon.cpp
    test_world.cpp
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
        ${BULLET_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
    )

target_include_directories(run_tests
    PRIVATE
        "${CMAKE_SOURCE_DIR}/tests"
        "${CMAKE_SOURCE_DIR}/rwgame"
    )

target_link_libraries(run_tests
    PRIVATE
        rwengine
        ${OPENGL_LIBRARIES}
        ${BULLET_LIBRARIES}
        ${SDL2_LIBRARY}
        ${PNG_LIBRARIES}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )

add_test(UnitTests run_tests)
