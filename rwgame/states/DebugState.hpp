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

    virtual void enter();
    virtual void exit();

    virtual void tick(float dt);
    virtual void draw(GameRenderer* r);

    virtual void handleEvent(const SDL_Event& event);

    void printCameraDetails();

    void spawnVehicle(unsigned int id);
    void spawnFollower(unsigned int id);
    void giveItem(int slot);

    const ViewCamera& getCamera();
};

#endif  // DEBUGSTATE_HPP
