#ifndef _RWENGINE_PAYPHONE_HPP_
#define _RWENGINE_PAYPHONE_HPP_

#include <glm/glm.hpp>
#include <string>

class PlayerController;

class GameWorld;
class GameState;

class GameObject;
class InstanceObject;
class CharacterObject;

class Payphone {
private:
    InstanceObject* object;
    glm::vec3 position;
    GameWorld* engine;
    float callTimer = 0.f;
    std::string message;

public:
    enum class State { Idle, Ringing, PickingUp, Talking, HangingUp };

    State state = State::Idle;

    int id;

    int getScriptObjectID() const {
        return id;
    }

    Payphone(GameWorld* engine_, const int id_, const glm::vec2 coord);
    ~Payphone();

    // Makes a payphone ring
    void enable();
    // Disables ringing
    void disable();
    // Is currently used by player
    bool isTalking() const;
    // Sets a message and makes a payphone ring
    void setMessageAndStartRinging(const std::string& m);
    void tick(float dt);
};

#endif