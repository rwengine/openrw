#include "RWImGui.hpp"

#include <ai/CharacterController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

#include "RWGame.hpp"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include <gl/gl_core_3_3.h>
#include <glm/gtx/norm.hpp>

RWImGui::RWImGui(GameBase& game) : _game(game) {
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

bool RWImGui::processEvent(SDL_Event& event) {
    if (!_context) {
        return false;
    }
    ImGui::SetCurrentContext(_context);
    ImGui_ImplSDL2_ProcessEvent(&event);
    auto& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void RWImGui::startFrame() {
    if (!_context) {
        return;
    }
    ImGui::SetCurrentContext(_context);

    auto [window, sdl_glcontext] = _game.getWindow().getSDLContext();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void RWImGui::endFrame() {
    static bool show_demo_window = false;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
