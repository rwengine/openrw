# Override CMake's FindBullet module:
# - create a bullet::bullet TARGET

include("${CMAKE_ROOT}/Modules/FindBullet.cmake")

if(BULLET_FOUND AND NOT TARGET bullet::bullet)
    add_library(bullet INTERFACE)
    target_link_libraries(bullet
        INTERFACE
            ${BULLET_LIBRARIES}
        )
    target_include_directories(bullet SYSTEM
        INTERFACE
            "${BULLET_INCLUDE_DIR}"
        )
    add_library(bullet::bullet ALIAS bullet)
endif()
