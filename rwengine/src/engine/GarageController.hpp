#ifndef _RWENGINE_GARAGECONTROLLER_HPP_
#define _RWENGINE_GARAGECONTROLLER_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <rw/defines.hpp>

class PlayerController;

struct GarageInfo;

class GameWorld;

class CharacterObject;
class InstanceObject;
class GameObject;

class GarageController {
private:
    float garageTimer = 0.f;

    bool swingType;

    glm::vec3 startPosition;
    glm::quat startRotation;

    float fraction;
    float step;
    float doorHeight;

    float getDistanceToGarage(glm::vec3 point);
    bool isObjectInsideGarage(GameObject* object);

    bool shouldOpen();
    bool shouldClose();
    bool shouldStopOpening();
    bool shouldStopClosing();

    void doOnOpenEvent();
    void doOnCloseEvent();
    void doOnStartOpeningEvent();
    void doOnStartClosingEvent();

public:
    GameWorld* engine;
    GarageInfo* garageInfo;
    InstanceObject* doorObject;

    GarageController(GameWorld* engine, GarageInfo* info, InstanceObject* door);
    ~GarageController();

    void tick(float dt);
};

#endif