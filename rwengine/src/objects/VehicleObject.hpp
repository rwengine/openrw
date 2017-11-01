#ifndef _RWENGINE_VEHICLEOBJECT_HPP_
#define _RWENGINE_VEHICLEOBJECT_HPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <data/ModelData.hpp>
#include <objects/GameObject.hpp>
#include <objects/VehicleInfo.hpp>

class Atomic;
class CharacterObject;
class CollisionInstance;
class GameWorld;
class ModelFrame;

class btCollisionShape;
struct btVehicleRaycaster;
class btRaycastVehicle;
class btRigidBody;
class btHingeConstraint;

/**
 * @class VehicleObject
 * Implements Vehicle behaviours.
 */
class VehicleObject : public GameObject, public ClumpObject {
private:
    float steerAngle;
    float throttle;
    float brake;
    bool handbrake;

    Atomic* chassishigh_ = nullptr;
    Atomic* chassislow_ = nullptr;
public:
    VehicleInfoHandle info;
    glm::u8vec3 colourPrimary;
    glm::u8vec3 colourSecondary;
    bool mHasSpecial;

    std::map<size_t, GameObject*> seatOccupants;

    std::unique_ptr<CollisionInstance> collision;
    btVehicleRaycaster* physRaycaster;
    btRaycastVehicle* physVehicle;

    struct Part {
        ModelFrame* dummy;
        Atomic* normal;
        Atomic* damaged;
        btCollisionShape *cs;
        btRigidBody* body;
        btHingeConstraint* constraint;
        bool moveToAngle;
        float targetAngle;
        float openAngle;
        float closedAngle;
    };

    std::map<std::string, Part> dynamicParts;

    VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot,
                  BaseModelInfo* modelinfo, VehicleInfoHandle info,
                  const glm::u8vec3& prim, const glm::u8vec3& sec);

    ~VehicleObject() override;

    void setPosition(const glm::vec3& pos) override;

    void setRotation(const glm::quat& orientation) override;

    void updateTransform(const glm::vec3& pos, const glm::quat& rot) override;

    VehicleModelInfo* getVehicle() const {
        return getModelInfo<VehicleModelInfo>();
    }

    Atomic* getHighLOD() const { return chassishigh_; }
    Atomic* getLowLOD() const { return chassislow_; }

    Type type() const override {
        return Vehicle;
    }

    void setSteeringAngle(float);

    float getSteeringAngle() const;

    void setThrottle(float);

    float getThrottle() const;

    void setBraking(float);

    float getBraking() const;

    void setHandbraking(bool);

    bool getHandbraking() const;

    void tick(float dt) override;

    void tickPhysics(float dt);

    bool isFlipped() const;

    float getVelocity() const;

    void ejectAll();

    GameObject* getOccupant(size_t seat);

    void setOccupant(size_t seat, GameObject* occupant);

    /**
     * @brief canOccupantExit
     * @return true if the vehicle is currently exitable
     */
    bool canOccupantExit() const;

    /**
     * @brief isOccupantDriver
     * @param seat
     * @return True if the given seat is the driver's seat.
     */
    bool isOccupantDriver(size_t seat) const;

    glm::vec3 getSeatEntryPosition(size_t seat) const {
        auto pos = info->seats[seat].offset;
        pos -= glm::vec3(glm::sign(pos.x) * -0.81756252f, 0.34800607f,
                         -0.486281008f);
        return pos;
    }
    glm::vec3 getSeatEntryPositionWorld(size_t seat) const {
        return getPosition() + getRotation() * getSeatEntryPosition(seat);
    }

    Part* getSeatEntryDoor(size_t seat);

    bool takeDamage(const DamageInfo& damage) override;

    enum FrameState { OK, DAM, BROKEN };

    void setPartState(Part* part, FrameState state);

    void setPartLocked(Part* part, bool locked);

    void setPartTarget(Part* part, bool enable, float target);

    Part* getPart(const std::string& name);

    void applyWaterFloat(const glm::vec3& relPt);

    void setPrimaryColour(uint8_t color);
    void setSecondaryColour(uint8_t color);

    /**
    * @brief isStopped
    * @return True if the vehicle isn't moving
    */
    bool isStopped() const;

    /**
    * @brief collectSpecial
    * @return True if mHasSpecial was true opon calling
    */
    bool collectSpecial();

    void grantOccupantRewards(CharacterObject* character);

private:
    void registerPart(ModelFrame* mf);
    void createObjectHinge(Part* part);
    void destroyObjectHinge(Part* part);
};

#endif
