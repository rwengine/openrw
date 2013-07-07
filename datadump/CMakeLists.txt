add_executable(datadump main.cpp)
include_directories(../framework2/include)
target_link_libraries( datadump renderware )

install(TARGETS datadump RUNTIME DESTINATION bin)
