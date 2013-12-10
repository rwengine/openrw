
FILE(GLOB TEST_SOURCES "*.cpp")
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 

add_executable(run_tests ${TEST_SOURCES}) 

include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/engine/include) 

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(../framework2/include /usr/include/bullet)

target_link_libraries(run_tests renderware sfml-window sfml-system GL GLEW BulletDynamics BulletCollision LinearMath ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(UnitTests run_tests)
