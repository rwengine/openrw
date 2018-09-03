add_library(microprofile STATIC
    microprofile/microprofile.h
    microprofile/microprofile_html.h
    microprofile/microprofile.cpp
)

target_include_directories(microprofile
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/microprofile"
)

target_compile_definitions(microprofile
    PUBLIC
        MICROPROFILE_GPU_TIMERS=0
)

find_package(Threads REQUIRED)
target_link_libraries(microprofile
    PUBLIC
        Threads::Threads
)

add_library(microprofile::microprofile ALIAS microprofile)
