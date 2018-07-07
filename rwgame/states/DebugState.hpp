#ifndef DEBUGSTATE_HPP
#define DEBUGSTATE_HPP

#include "State.hpp"

class DebugState final : public State {
    ViewCamera _debugCam;
    glm::vec3 _movement{};
    glm::vec2 _debugLook{};
    bool _freeLook = false;
    bool _sonicMode = false;
    bool _invertedY;

    std::shared_ptr<Menu> createDebugMenu();
    std::shared_ptr<Menu> createMapMenu();
    std::shared_ptr<Menu> createVehicleMenu();
    std::shared_ptr<Menu> createAIMenu();
    std::shared_ptr<Menu> createWeaponMenu();
    std::shared_ptr<Menu> createWeatherMenu();
    std::shared_ptr<Menu> createMissionsMenu();

public:
    DebugState(RWGame* game, const glm::vec3& vp = {},
               const glm::quat& vd = {1.0f, 0.0f, 0.0f, 0.0f});

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer* r) override;

    void handleEvent(const SDL_Event& event) override;

    void printCameraDetails();

    void spawnVehicle(unsigned int id);
    void spawnFollower(unsigned int id);
    void giveItem(int slot);

    const ViewCamera& getCamera(float) override;
};

#endif  // DEBUGSTATE_HPP
