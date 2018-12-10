#ifndef _RWENGINE_GARAGE_HPP_
#define _RWENGINE_GARAGE_HPP_

#include <glm/vec3.hpp>
#include <rw/debug.hpp>

namespace ai {
class PlayerController;
}  // namespace ai

class GameWorld;

class CharacterObject;
class InstanceObject;
class GameObject;

// Garage types are from original game
enum class GarageType {
    Mission = 1,
    BombShop1 = 2,
    BombShop2 = 3,
    BombShop3 = 4,
    Respray = 5,
    CollectCars1 = 8,
    CollectCars2 = 9,
    MissionForCarToComeOut = 11,
    Crusher = 13,
    MissionKeepCar = 14,
    Hideout1 = 16,
    Hideout2 = 17,
    Hideout3 = 18,
    MissionToOpenAndClose = 19,
    MissionForSpecificCar = 20,
    MissionKeepCarAndRemainClosed = 21,
};

enum class GarageState { Closed, Closing, Opening, Opened };

class Garage {
private:
    InstanceObject* doorObject = nullptr;
    InstanceObject* secondDoorObject = nullptr;

    bool active = true;

    float garageTimer = 0.f;

    bool swingType = false;

    glm::vec3 startPosition;
    glm::vec3 startPositionSecondDoor;

    bool needsToUpdate = false;

    float fraction = 0.f;
    float step = 3.f;  // this should be adjusted somehow
                       // to look similar to original game
    float doorHeight = 4.f;

    float getDistanceToGarage(glm::vec3 point);

    bool shouldOpen();
    bool shouldClose();
    bool shouldStopOpening();
    bool shouldStopClosing();

    void doOnOpenEvent();
    void doOnCloseEvent();
    void doOnStartOpeningEvent();
    void doOnStartClosingEvent();

    void updateDoor();

public:
    GameWorld* engine;
    size_t id;

    size_t getScriptObjectID() const {
        return id;
    }

    glm::vec3 min;
    glm::vec3 max;

    GarageType type;

    GameObject* target = nullptr;
    // @todo use model type
    int targetModel = 0;

    GarageState state = GarageState::Closed;

    bool resprayDone = false;

    Garage(GameWorld* engine_, size_t id_, const glm::vec3& coord0,
               const glm::vec3& coord1, GarageType type_);
    ~Garage() = default;

    void makeDoorSwing();

    bool isObjectInsideGarage(GameObject* object) const;
    bool isTargetInsideGarage() const;

    void activate();
    void deactivate();

    void open();
    void close();

    void tick(float dt);
};

#endif
