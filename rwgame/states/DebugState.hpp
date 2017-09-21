#ifndef DEBUGSTATE_HPP
#define DEBUGSTATE_HPP

#include "State.hpp"

class DebugState : public State {
    ViewCamera _debugCam;
    glm::vec3 _movement;
    glm::vec2 _debugLook;
    bool _freeLook = false;
    bool _sonicMode = false;
    bool _invertedY;

    std::shared_ptr<Menu> createDebugMenu();
    std::shared_ptr<Menu> createMapMenu();
    std::shared_ptr<Menu> createVehicleMenu();
    std::shared_ptr<Menu> createAIMenu();
    std::shared_ptr<Menu> createWeaponMenu();

public:
    DebugState(RWGame* game, const glm::vec3& vp = {},
               const glm::quat& vd = {});

    virtual void enter() override;
    virtual void exit() override;

    virtual void tick(float dt) override;
    virtual void draw(GameRenderer* r) override;

    virtual void handleEvent(const SDL_Event& event) override;

    void printCameraDetails();

    void spawnVehicle(unsigned int id);
    void spawnFollower(unsigned int id);
    void giveItem(int slot);

    const ViewCamera& getCamera(float) override;
};

#endif  // DEBUGSTATE_HPP
