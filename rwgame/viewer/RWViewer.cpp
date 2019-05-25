#include "RWViewer.hpp"

#include "GameInput.hpp"
#include "RWImGui.hpp"

#include <core/Logger.hpp>
#include <core/Profiler.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <array>

#include <string.h>

namespace {
static constexpr std::array<
    std::tuple<GameRenderer::SpecialModel, char const*, char const*>, 3>
    kSpecialModels{{{GameRenderer::ZoneCylinderA, "zonecyla.dff", "particle"},
                    {GameRenderer::ZoneCylinderB, "zonecylb.dff", "particle"},
                    {GameRenderer::Arrow, "arrow.dff", ""}}};
const auto kViewerFov = glm::radians(90.f);
}  // namespace

RWViewer::RWViewer(Logger& log, const std::optional<RWArgConfigLayer>& args)
    : GameBase(log, args)
    , data_(&log, config.gamedataPath())
    , renderer_(&log, &data_)
    , imgui_(*this) {
    RW_PROFILE_THREAD("Main");
    RW_TIMELINE_ENTER("Startup", MP_YELLOW);

    log.info("Game", "Game directory: " + config.gamedataPath());

    if (!GameData::isValidGameDirectory(config.gamedataPath())) {
        throw std::runtime_error("Invalid game directory path: " +
                                 config.gamedataPath());
    }

    imgui_.init();

    data_.load();
    textViewer_.emplace(data_);

    for (const auto& [specialModel, fileName, name] : kSpecialModels) {
        auto model = data_.loadClump(fileName, name);
        renderer_.setSpecialModel(specialModel, model);
    }

    // Set up text renderer_
    renderer_.text.setFontTexture(FONT_PAGER, "pager");
    renderer_.text.setFontTexture(FONT_PRICEDOWN, "font1");
    renderer_.text.setFontTexture(FONT_ARIAL, "font2");

    data_.loadDynamicObjects(
        (rwfs::path{config.gamedataPath()} / "data/object.dat")
            .string());  // FIXME: use path

    data_.loadGXT("text/" + config.gameLanguage() + ".gxt");

    world_ = std::make_unique<GameWorld>(&log, &data_);
    world_->state = &state_;

    log.info("Game", "Started");
    RW_TIMELINE_LEAVE("Startup");
}

RWViewer::~RWViewer() {
    log.info("Game", "Beginning cleanup");
}

int RWViewer::run() {
    namespace chrono = std::chrono;

    auto lastFrame = chrono::steady_clock::now();

    while (continue_) {
        RW_PROFILE_FRAME_BOUNDARY();
        RW_PROFILE_SCOPE("Main Loop");

        continue_ = updateInput();

        auto currentFrame = chrono::steady_clock::now();
        auto frameTime =
            chrono::duration<float>(currentFrame - lastFrame).count();
        lastFrame = currentFrame;

        render(1, frameTime);

        getWindow().swap();
    }

    return 0;
}

bool RWViewer::updateInput() {
    RW_PROFILE_SCOPE(__func__);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (imgui_.processEvent(event)) {
            continue;
        }
        switch (event.type) {
            case SDL_QUIT:
                return false;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        break;
                }
                break;

            case SDL_KEYDOWN:
                globalKeyEvent(event);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == 1) mouseMode_ = MouseMode::Dragging;
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == 1) mouseMode_ = MouseMode::Hovering;
                break;

            case SDL_MOUSEWHEEL:
                viewParams_.z =
                    glm::max(1.f, viewParams_.z +
                                      -(event.wheel.y * viewParams_.z * 0.1f));
                break;

            case SDL_MOUSEMOTION:
                if (mouseMode_ == MouseMode::Dragging) {
                    viewParams_.x += event.motion.xrel * 0.5f;
                    viewParams_.y += event.motion.yrel * 0.5f;
                }
                break;
        }
    }
    return true;
}

void RWViewer::tick(float dt) {
    RW_UNUSED(dt);
    RW_PROFILE_SCOPE(__func__);
}

void RWViewer::render(float alpha, float time) {
    RW_PROFILE_SCOPEC(__func__, MP_CORNFLOWERBLUE);
    RW_UNUSED(time);

    getRenderer().getRenderer().swap();
    imgui_.startFrame();

    glm::ivec2 windowSize = getWindow().getSize();
    renderer_.setViewport(windowSize.x, windowSize.y);

    ViewCamera viewCam{};
    viewCam.frustum.fov = kViewerFov;

    if (viewedObject_) {
        auto rotation = glm::angleAxis(glm::radians(viewParams_.x),
                                       glm::vec3(0.f, 0.f, -1.f)) *
                        glm::angleAxis(glm::radians(viewParams_.y),
                                       glm::vec3(0.f, 1.f, 0.f));
        auto viewDirection = rotation * glm::vec3{-1.f, 0.f, 0.f};
        viewCam.position = viewDirection * viewParams_.z;
        viewCam.rotation = rotation;
    }

    viewCam.frustum.aspectRatio =
        windowSize.x / static_cast<float>(windowSize.y);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    renderer_.getRenderer().pushDebugGroup("World");

    renderer_.renderWorld(world_.get(), viewCam, alpha);

    renderer_.getRenderer().popDebugGroup();

    drawWindows();
    drawMenu();

    imgui_.endFrame();
}

void RWViewer::globalKeyEvent(const SDL_Event& event) {
    switch (event.key.keysym.sym) {
        case SDLK_LEFTBRACKET:
            world_->offsetGameTime(-30);
            break;
        case SDLK_RIGHTBRACKET:
            world_->offsetGameTime(30);
            break;
        default:
            break;
    }
}

void RWViewer::drawMenu() {
    auto menu = [](const char* name, auto then) {
        if (ImGui::BeginMenu(name)) then(), ImGui::EndMenu();
    };

    if (ImGui::BeginMainMenuBar()) {
        menu("File", [&]() {
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                continue_ = false;
            }
        });
        menu("View", [&]() {
            ImGui::MenuItem("Model", "", viewedObject_ != nullptr);
            if (ImGui::MenuItem("Remove Object", "")) {
                if (viewedObject_) {
                    world_->destroyObject(viewedObject_);
                    viewedObject_ = nullptr;
                }
            }
        });
        menu("Windows", [&]() {
            ImGui::MenuItem("Models", "", &showModelList_);
            ImGui::MenuItem("Texts", "", &showTextViewer_);
            ImGui::Separator();
            ImGui::MenuItem("Demo", "", &showImGuiDemo_);
        });
        ImGui::EndMainMenuBar();
    }
}

void RWViewer::drawWindows() {
    auto window = [](const char* name, bool& openWindow, const ImVec2& size,
                     auto then) {
        if (!openWindow) return;
        ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
        if (ImGui::Begin(name, &openWindow)) then();
        ImGui::End();
    };

    window("Models", showModelList_, {400, 180}, [&]() {
        static ImGuiTextFilter filter;

        filter.Draw();
        {
            ImGui::BeginChild("ModelList", ImVec2(0, 0), false);
            ImGui::Columns(3);

            ImGui::Text("Model");
            ImGui::NextColumn();
            ImGui::Text("ID");
            ImGui::NextColumn();
            ImGui::Text("Type");
            ImGui::NextColumn();
            ImGui::Separator();

            auto& models = data_.modelinfo;
            for (const auto& [id, info] : models) {
                if (filter.PassFilter(info->name.c_str()) ||
                    filter.PassFilter(std::to_string(id).c_str())) {
                    bool selected = false;
                    ImGui::PushID(id);
                    if (ImGui::Selectable(
                            info->name.c_str(), &selected,
                            ImGuiSelectableFlags_SpanAllColumns |
                                ImGuiSelectableFlags_AllowDoubleClick))
                        if (ImGui::IsMouseDoubleClicked(0)) viewModel(id);
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Details")) showModel(id);
                        if (ImGui::MenuItem("View")) viewModel(id);
                        ImGui::EndPopup();
                    }
                    ImGui::NextColumn();
                    ImGui::Text("%u", id);
                    ImGui::NextColumn();
                    ImGui::Text(
                        "%s", BaseModelInfo::getTypeName(info->type()).c_str());
                    ImGui::NextColumn();
                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
        }
    });

    for (auto& id : showModels_) drawModelWindow(id);

    window("Texts", showTextViewer_, {920, 320},
           [&]() { textViewer_->draw(renderer_); });

    if (showImGuiDemo_) ImGui::ShowDemoWindow(&showImGuiDemo_);
}

void RWViewer::drawModelWindow(ModelID id) {
    std::stringstream ss;
    ss << "Model " << id;

    bool open = true;
    ImGui::Begin(ss.str().c_str(), &open, ImGuiWindowFlags_NoCollapse);
    if (!open) showModels_.erase(id);

    auto it = data_.modelinfo.find(id);
    if (it == data_.modelinfo.end()) {
        ImGui::Text("Invalid Model");
        ImGui::End();
        return;
    }
    const auto& info = it->second;

    ImGui::Text("%s", info->name.c_str());
    ImGui::Text("%s", BaseModelInfo::getTypeName(info->type()).c_str());
    if (info->type() == ModelDataType::VehicleInfo) {
        auto& vehicle = static_cast<VehicleModelInfo&>(*info);
        ImGui::Text("Vehicle Name: %s", vehicle.vehiclename_.c_str());
    }

    ImGui::End();
}

void RWViewer::viewModel(ModelID model) {
    if (viewedObject_) {
        world_->destroyObject(viewedObject_);
        viewedObject_ = nullptr;
    }

    auto it = data_.modelinfo.find(model);
    if (it == data_.modelinfo.end()) {
        return;
    }
    const auto& info = it->second;

    if (info->type() == ModelDataType::VehicleInfo) {
        viewedObject_ = world_->createVehicle(model, {}, {1.f, 0.f, 0.f, 0.f});
    }
    if (info->type() == ModelDataType::SimpleInfo) {
        viewedObject_ = world_->createInstance(model, {}, {1.f, 0.f, 0.f, 0.f});
    }
    if (info->type() == ModelDataType::PedInfo) {
        viewedObject_ =
            world_->createPedestrian(model, {}, {1.f, 0.f, 0.f, 0.f});
    }

    float radius = 10.f;
    if (auto clump = viewedObject_->getClump(); clump)
        radius = clump->getBoundingRadius();
    if (auto atomic = viewedObject_->getAtomic(); atomic) {
        auto& geo = atomic->getGeometry()->geometryBounds;
        radius = glm::length(geo.center) + geo.radius;
    }
    constexpr auto kViewSlop = 1.f;
    viewParams_.z = kViewSlop * ((radius * 2) / (glm::tan(kViewerFov / 2.f)));
}
