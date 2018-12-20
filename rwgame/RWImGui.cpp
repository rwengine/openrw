#include "RWImGui.hpp"

#ifdef RW_IMGUI

#include "RWGame.hpp"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <gl/gl_core_3_3.h>

RWImGui::RWImGui(RWGame &game)
    : _game(game) {
}

RWImGui::~RWImGui() {
    destroy();
}

void RWImGui::init() {
    IMGUI_CHECKVERSION();
    _context = ImGui::CreateContext();

    auto [window, context] = _game.getWindow().getSDLContext();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void RWImGui::destroy() {
    if (!_context) {
        return;
    }
    ImGui::SetCurrentContext(_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    _context = nullptr;
}

bool RWImGui::process_event(SDL_Event &event) {
    if (!_context) {
        return false;
    }
    ImGui::SetCurrentContext(_context);
    ImGui_ImplSDL2_ProcessEvent(&event);
    auto& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void RWImGui::tick() {
    if (!_context) {
        return;
    }
    ImGui::SetCurrentContext(_context);
    auto& io = ImGui::GetIO();

    SDL_Window *window;
    std::tie(window, std::ignore) = _game.getWindow().getSDLContext();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    static float f = 0.0f;

    ImGui::Begin("Hello, world!");
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / io.Framerate), static_cast<double>(io.Framerate));   ImGui::End();

    static bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static ImGuiIO *g_io;

static SDL_Window *g_sdl_window;
static SDL_GLContext g_sdl_gl_context;


void imgui_init(SDL_Window *sdl_window, SDL_GLContext sdl_gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    g_io = &ImGui::GetIO();

    g_sdl_window = sdl_window;
    g_sdl_gl_context = sdl_gl_context;

    ImGui_ImplSDL2_InitForOpenGL(sdl_window, sdl_gl_context);
    ImGui_ImplOpenGL3_Init("#version 150");

//    ImGui::StyleColorsDark();
//    ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();


    // Build atlas
    unsigned char* tex_pixels = NULL;
    int tex_w, tex_h;
    g_io->Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
}

void imgui_process_event(SDL_Event *event, bool *mouse, bool *keyboard) {
    ImGui_ImplSDL2_ProcessEvent(event);
    *mouse = g_io->WantCaptureMouse;
    *keyboard = g_io->WantCaptureKeyboard;
}

static bool show_demo_window = true;

void imgui_tick() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(g_sdl_window);
    ImGui::NewFrame();

//    g_io->DisplaySize = ImVec2(1920, 1080);
//    g_io->DeltaTime = 1.0f / 60.0f;

    static float f = 0.0f;

    ImGui::Begin("Hello, world!");
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / g_io->Framerate), static_cast<double>(g_io->Framerate));
    ImGui::End();

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

//    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Render();
//    SDL_GL_MakeCurrent(g_sdl_window, g_sdl_gl_context);
//    glViewport(0, 0, static_cast<int>(g_io->DisplaySize.x), static_cast<int>(g_io->DisplaySize.y));
//    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
//    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//    SDL_GL_SwapWindow(g_sdl_window);
}

void imgui_destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    g_io = nullptr;
    g_sdl_gl_context = nullptr;
    g_sdl_window = nullptr;
}

#else

RWImGui::RWImGui(RWGame &game)
    : _game(game) {
}

RWImGui::~RWImGui() {
    destroy();
}

RWImGui::init(SDL_Window *window, SDL_GLContext context) {
}

RWImGui::destroy() {
}

RWImGui::process_event(SDL_Event &event) {
}

#endif
