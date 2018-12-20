add_library(imgui EXCLUDE_FROM_ALL
    rw_imconfig.h
    imgui/imgui.h
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)

target_include_directories(imgui SYSTEM
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/imgui"
)

target_compile_definitions(imgui
    PUBLIC
        IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/rw_imconfig.h"
)

add_library(imgui::core ALIAS imgui)

openrw_target_apply_options(
    TARGET imgui
)

add_library(imgui_sdl_gl3 EXCLUDE_FROM_ALL
    imgui/examples/imgui_impl_opengl3.h
    imgui/examples/imgui_impl_opengl3.cpp
    imgui/examples/imgui_impl_sdl.h
    imgui/examples/imgui_impl_sdl.cpp
)

target_include_directories(imgui_sdl_gl3 SYSTEM
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/imgui/examples"
)

target_link_libraries(imgui_sdl_gl3
    PUBLIC
        imgui::core
        SDL2::SDL2
)

# FIXME: extract gl loader to target + add property to get header
target_compile_definitions(imgui_sdl_gl3
    PRIVATE
        "IMGUI_IMPL_OPENGL_LOADER_CUSTOM=\"${OpenRW_SOURCE_DIR}/rwcore/gl/gl_core_3_3.h\""
)

add_library(imgui::sdl_gl3 ALIAS imgui_sdl_gl3)

openrw_target_apply_options(
    TARGET imgui_sdl_gl3
)
