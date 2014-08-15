FILE(GLOB TEST_SOURCES "*.cpp")
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 

add_executable(run_tests ${TEST_SOURCES}) 

include_directories(include)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/rwengine/include" "${CMAKE_SOURCE_DIR}/rwgame" ${BULLET_INCLUDE_DIR})

target_link_libraries(run_tests rwengine sfml-window sfml-system sfml-graphics ${OPENGL_LIBRARIES} GLEW BulletDynamics BulletCollision LinearMath ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} iconv)

add_test(UnitTests run_tests)
