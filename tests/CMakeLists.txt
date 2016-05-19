##############################################################################
#    Unit Tests
##############################################################################

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(TEST_SOURCES
	"main.cpp"
	"test_animation.cpp"
	"test_archive.cpp"
	"test_buoyancy.cpp"
	"test_character.cpp"
	"test_chase.cpp"
	"test_cutscene.cpp"
	"test_data.cpp"
	"test_FileIndex.cpp"
	"test_GameData.cpp"
	"test_GameWorld.cpp"
	"test_globals.hpp"
	"test_items.cpp"
	"test_lifetime.cpp"
	"test_loaderdff.cpp"
	"test_Logger.cpp"
	"test_menu.cpp"
	"test_object.cpp"
	"test_object_data.cpp"
	"test_pickup.cpp"
	"test_renderer.cpp"
	"test_Resource.cpp"
	"test_rwbstream.cpp"
	"test_SaveGame.cpp"
	"test_scriptmachine.cpp"
	"test_skeleton.cpp"
	"test_state.cpp"
	"test_text.cpp"
	"test_trafficdirector.cpp"
	"test_vehicle.cpp"
	"test_VisualFX.cpp"
	"test_weapon.cpp"
	"test_worker.cpp"
	"test_world.cpp"
	)

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

add_executable(run_tests ${TEST_SOURCES})

include_directories(
	include
	"${CMAKE_SOURCE_DIR}/tests"
	"${CMAKE_SOURCE_DIR}/rwengine/include"
	"${CMAKE_SOURCE_DIR}/rwgame")

include_directories(SYSTEM
	${BULLET_INCLUDE_DIR}
	${SFML_INCLUDE_DIR})

target_link_libraries(run_tests
	rwengine
	${OPENGL_LIBRARIES}
	${SFML_LIBRARIES}
	${BULLET_LIBRARIES}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(UnitTests run_tests)
