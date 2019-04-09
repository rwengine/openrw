#ifndef DEBUGSTATE_HPP
#define DEBUGSTATE_HPP

#include "State.hpp"

#include <render/ViewCamera.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>

class DebugState final : public State {
    ViewCamera _debugCam;
    glm::vec3 _movement{};
    glm::vec2 _debugLook{};
    bool _freeLook = false;
    bool _sonicMode = false;
    bool _invertedY;

    Menu createDebugMenu();
    Menu createMapMenu();
    Menu createVehicleMenu();
    Menu createAIMenu();
    Menu createWeaponMenu();
    Menu createWeatherMenu();
    Menu createMissionsMenu();

public:
    DebugState(RWGame* game, const glm::vec3& vp = {},
               const glm::quat& vd = {1.0f, 0.0f, 0.0f, 0.0f});

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer &r) override;

    void handleEvent(const SDL_Event& event) override;

    void printCameraDetails();

    void spawnVehicle(unsigned int id);
    void spawnFollower(unsigned int id);
    void giveItem(int slot);

    const ViewCamera& getCamera(float) override;
};

#endif  // DEBUGSTATE_HPP
