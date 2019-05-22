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

namespace {
void WindowDebugStats(RWGame& game) {
    auto& io = ImGui::GetIO();

    auto time_ms = 1000.0f / io.Framerate;
    constexpr size_t average_every_frame = 240;
    static float times[average_every_frame];
    static size_t times_index = 0;
    static double time_average = 0, time_min = 0, time_max = 0;
    times[times_index++] = time_ms;
    if (times_index >= average_every_frame) {
        times_index = 0;
        time_average = 0;
        time_min = std::numeric_limits<double>::max();
        time_max = std::numeric_limits<double>::lowest();

        for (double time : times) {
            time_average += time;
            time_min = std::min(time, time_min);
            time_max = std::max(time, time_max);
        }
        time_average /= average_every_frame;
    }

    const auto& world = game.getWorld();
    auto& renderer = game.getRenderer();

    ImGui::SetNextWindowPos({20.f, 20.f});
    ImGui::Begin("Engine Information", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoInputs);
    ImGui::Text("%.3f ms/frame (%.1f FPS)\n%.3f / %.3f / %.3f ms",
                static_cast<double>(1000.0f / io.Framerate),
                static_cast<double>(io.Framerate), time_average, time_min,
                time_max);
    ImGui::Text("Timescale %.2f",
                static_cast<double>(world->state->basic.timeScale));
    ImGui::Text("%i Drawn %lu Culled", renderer.getRenderer().getDrawCount(),
                renderer.getCulledCount());
    ImGui::Text("%i Textures %i Buffers",
                renderer.getRenderer().getTextureCount(),
                renderer.getRenderer().getBufferCount());
    ImGui::End();
}

void WindowDebugObjects(RWGame& game, const ViewCamera& camera) {
    auto& data = game.getGameData();
    auto world = game.getWorld();

    ImGui::SetNextWindowPos({20.f, 20.f});
    ImGui::Begin("Object Information", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoInputs);
    ImGui::Text("%lu Models", data.modelinfo.size());
    ImGui::Text("Dynamic Objects\n %lu Vehicles\n %lu Peds",
                world->vehiclePool.objects.size(),
                world->pedestrianPool.objects.size());
    ImGui::End();

    // Render worldspace overlay for nearby objects
    constexpr float kNearbyDistance = 25.f;
    const auto& view = camera.position;
    const auto& model = camera.getView();
    const auto& proj = camera.frustum.projection();
    const auto& size = game.getWindow().getSize();
    glm::vec4 viewport(0.f, 0.f, size.x, size.y);
    auto isnearby = [&](GameObject* o) {
        return glm::distance2(o->getPosition(), view) <
               kNearbyDistance * kNearbyDistance;
    };
    auto showdata = [&](GameObject* o, std::stringstream& ss) {
        auto screen = glm::project(o->getPosition(), model, proj, viewport);
        if (screen.z >= 1.f) {
            return;
        }
        ImGui::SetNextWindowPos({screen.x, viewport.w - screen.y}, 0,
                                {0.5f, 0.5f});
        ImGui::Begin(
            std::to_string(reinterpret_cast<uintptr_t>(o)).c_str(), nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoInputs);
        ImGui::Text("%s", ss.str().c_str());
        ImGui::End();
    };

    for (auto& p : world->vehiclePool.objects) {
        if (!isnearby(p.second.get())) continue;
        auto v = static_cast<VehicleObject*>(p.second.get());

        std::stringstream ss;
        ss << v->getVehicle()->vehiclename_ << "\n"
           << (v->isFlipped() ? "Flipped" : "Upright") << "\n"
           << (v->isStopped() ? "Stopped" : "Moving") << "\n"
           << v->getVelocity() << "m/s\n";

        showdata(v, ss);
    }
    for (auto& p : world->pedestrianPool.objects) {
        if (!isnearby(p.second.get())) continue;
        auto c = static_cast<CharacterObject*>(p.second.get());
        const auto& state = c->getCurrentState();
        auto act = c->controller->getCurrentActivity();

        std::stringstream ss;
        ss << "Health: " << state.health << " (" << state.armour << ")\n"
           << (c->isAlive() ? "Alive" : "Dead") << "\n"
           << "Activity: " << (act ? act->name() : "Idle") << "\n";

        showdata(c, ss);
    }
}
}  // namespace

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

void RWImGui::endFrame(const ViewCamera& camera) {
    switch (_game.getDebugViewMode()) {
        case RWGame::DebugViewMode::General:
            WindowDebugStats(_game);
            break;
        case RWGame::DebugViewMode::Objects:
            WindowDebugObjects(_game, camera);
            break;
        default:
            break;
    }

    static bool show_demo_window = false;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
