##############################################################################
#    Unit Tests
##############################################################################

find_package(Boost COMPONENTS filesystem unit_test_framework system REQUIRED)

set(TESTS
    Animation
    Archive
    Buoyancy
    Character
    Chase
    Config
    Cutscene
    Data
    FileIndex
    GameData
    GameWorld
    Input
    Items
    Lifetime
    LoaderDFF
    LoaderIPL
    Logger
    Menu
    Object
    ObjectData
    Pickup
    Renderer
    RWBStream
    SaveGame
    ScriptMachine
    State
    Text
    TrafficDirector
    Vehicle
    VisualFX
    Weapon
    World
    ZoneData
    )

set(TEST_SOURCES
    main.cpp
    test_Globals.cpp
    test_Globals.hpp

    # Hack in rwgame sources until there's a per-target test suite
    "${CMAKE_SOURCE_DIR}/rwgame/GameConfig.cpp"
    "${CMAKE_SOURCE_DIR}/rwgame/GameWindow.cpp"
    "${CMAKE_SOURCE_DIR}/rwgame/GameInput.cpp"
    )

foreach(TEST ${TESTS})
    list(APPEND TEST_SOURCES "test_${TEST}.cpp")
endforeach()

add_executable(run_tests
    ${TEST_SOURCES}
    )

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

if(SEPERATE_TEST_SUITES)
    foreach(TEST ${TESTS})
        add_test(
            NAME "${TEST}"
            COMMAND "$<TARGET_FILE:run_tests>" "-t" "${TEST}Tests"
            )
        set_tests_properties("${TEST}"
            PROPERTIES
                TIMEOUT 15
            )
    endforeach()
else()
    add_test(NAME UnitTests
        COMMAND "$<TARGET_FILE:run_tests>"
        )
    set_tests_properties(UnitTests
        PROPERTIES
            TIMEOUT 60
        )
endif()
