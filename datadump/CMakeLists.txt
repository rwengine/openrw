add_executable(datadump main.cpp)
include_directories(../rwengine/include)
target_link_libraries( datadump rwengine )

install(TARGETS datadump RUNTIME DESTINATION bin)
