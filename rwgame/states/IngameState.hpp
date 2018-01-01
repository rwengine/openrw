#ifndef INGAMESTATE_HPP
#define INGAMESTATE_HPP

#include "StateManager.hpp"

class PlayerController;

class IngameState : public State {
    enum CameraMode {
        CAMERA_CLOSE = 0,
        CAMERA_NORMAL = 1,
        CAMERA_FAR = 2,
        CAMERA_TOPDOWN = 3,
        /** Used for counting - not a valid camera mode */
        CAMERA_MAX
    };

    bool started;
    std::string save;
    bool newgame;
    ViewCamera _look;
    glm::vec3 cameraPosition;
    /** Timer to hold user camera position */
    float autolookTimer;
    CameraMode camMode;

    /// Player camera input since the last update
    glm::vec2 cameradelta_;
    /// Invert Y axis movement
    bool m_invertedY;
    /// Free look in vehicles.
    bool m_vehicleFreeLook;

    float moneyTimer = 0.f;  // Timer used to updated displayed money value

public:
    /**
     * @brief IngameState
     * @param game
     * @param newgame
     * @param game An empty string, a save game to load, or the string "test".
     */
    IngameState(RWGame* game, bool newgame = true,
                const std::string& save = "");

    void startTest();
    void startGame();

    void enter() override;

    void exit() override;

    void tick(float dt) override;

    void draw(GameRenderer* r) override;

    void handleEvent(const SDL_Event& event) override;
    virtual void handlePlayerInput(const SDL_Event& event);

    bool shouldWorldUpdate() override;

    const ViewCamera& getCamera(float alpha) override;
private:
    GameObject* getCameraTarget() const;
    float getViewDistance() const;
};

#endif  // INGAMESTATE_HPP
