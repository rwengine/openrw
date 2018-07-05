#ifndef _RWENGINE_CHASE_HPP_
#define _RWENGINE_CHASE_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include <string>
#include <vector>

class GameObject;

struct ChaseKeyframe {
    glm::vec3 velocity;
    int steeringAngle;
    int acceleratorPower;
    int brakePower;
    bool handbrake;
    glm::vec3 position;
    glm::quat rotation;

    ChaseKeyframe(glm::vec3 _velocity, int _steeringAngle, int _acceleratorPower, int _brakePower, bool _handbrake, glm::vec3 _position, glm::quat _rotation)
        : velocity(_velocity)
        , steeringAngle(_steeringAngle)
        , acceleratorPower(_acceleratorPower)
        , brakePower(_brakePower)
        , handbrake(_handbrake)
        , position(_position)
        , rotation(_rotation) {
    }

    static bool load(const std::string& filePath,
                     std::vector<ChaseKeyframe>& frames);
};

/**
 * @brief The ChaseCoordinator class handles loading and playing a chase
 *
 * It reads in ChaseKeyframes for a set of objects, and replays them
 * over time.
 */
class ChaseCoordinator {
public:
    ChaseCoordinator() = default;

    bool addChaseVehicle(GameObject* vehicle, int index,
                         const std::string& pathFile);
    GameObject* getChaseVehicle(int index);
    void removeChaseVehicle(int index);

    void start();
    void update(float dt);

    void cleanup();

private:
    float chaseTime{-1.f};
    struct ChaseObject {
        std::vector<ChaseKeyframe> keyframes;
        GameObject* object;
    };

    std::map<int, ChaseObject> chaseVehicles;
};

#endif
