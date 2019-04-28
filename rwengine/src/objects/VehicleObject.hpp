#ifndef _RWENGINE_VEHICLEOBJECT_HPP_
#define _RWENGINE_VEHICLEOBJECT_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <LinearMath/btScalar.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include <glm/gtc/type_precision.hpp>
#include <glm/vec3.hpp>

#include <data/ModelData.hpp>
#include <objects/GameObject.hpp>
#include <objects/Vehicle/AutoPilot.hpp>
#include <objects/VehicleInfo.hpp>

class Atomic;
class CharacterObject;
class CollisionInstance;
class GameWorld;
class ModelFrame;
class btRaycastVehicle;
class btCollisionShape;
class btRigidBody;
class btHingeConstraint;
class btMotionState;
struct btVehicleRaycaster;

enum CarLock : int32_t {
    CARLOCK_NONE = 0,
    CARLOCK_UNLOCKED = 1,
    CARLOCK_LOCKED = 2,
    CARLOCK_LOCKOUT_PLAYER_ONLY = 3,
    CARLOCK_LOCKED_PLAYER_INSIDE = 4,
    CARLOCK_LOCKED_INITIALLY = 5,
    CARLOCK_FORCE_SHUT_DOORS = 6,
    CARLOCK_LOCKED_BUT_CAN_BE_DAMAGED = 7,
};

enum RadioStation : int8_t {
    HEAD_RADIO = 0x0,
    DOUBLE_CLEF = 0x1,
    JAH_RADIO = 0x2,
    RISE_FM = 0x3,
    LIPS_106 = 0x4,
    GAME_FM = 0x5,
    MSX_FM = 0x6,
    FLASHBACK = 0x7,
    CHATTERBOX = 0x8,
    USERTRACK = 0x9,
    POLICE_RADIO = 0xA,
    RADIO_OFF = 0xB,
};

enum VehicleType {
    VEHICLE_AUTOMOBILE = 0x0,
    VEHICLE_BOAT = 0x1,
    VEHICLE_TRAIN = 0x2,
    VEHICLE_HELI = 0x3,
    VEHICLE_PLANE = 0x4,
    VEHICLE_BIKE = 0x5,
};

/**
 * @class VehicleObject
 * Implements Vehicle behaviours.
 */
class VehicleObject final : public GameObject, public ClumpObject {
private:
    float steerAngle{0.f};
    float throttle{0.f};
    float brake{0.f};
    bool handbrake = true;
    std::vector<btScalar> wheelsRotation;

    Atomic* chassishigh_ = nullptr;
    Atomic* chassislow_ = nullptr;

    std::array<Atomic*, 6> extras_{};

public:
    float health{1000.f};

    VehicleInfo* info = nullptr;
    glm::u8vec3 colourPrimary{};
    glm::u8vec3 colourSecondary{};
    bool mHasSpecial = true;

    std::map<size_t, GameObject*> seatOccupants;

    std::unique_ptr<CollisionInstance> collision;
    std::unique_ptr<btVehicleRaycaster> physRaycaster;
    std::unique_ptr<btRaycastVehicle> physVehicle;

    struct Part {
        Part(ModelFrame* p_dummy, Atomic* p_normal, Atomic* p_damaged,
             std::unique_ptr<btCollisionShape> p_cs,
             std::unique_ptr<btRigidBody> p_body,
             std::unique_ptr<btHingeConstraint> p_constraint,
             bool p_moveToAngle, float p_targetAngle, float p_openAngle,
             float p_closedAngle);

        Part(Part&& part) = default;
        Part& operator=(Part&& part) = default;

        ~Part() = default;

        ModelFrame* dummy;
        Atomic* normal;
        Atomic* damaged;
        std::unique_ptr<btCollisionShape> cs;
        std::unique_ptr<btRigidBody> body;
        std::unique_ptr<btMotionState> motionState;
        std::unique_ptr<btHingeConstraint> constraint;
        bool moveToAngle;
        float targetAngle;
        float openAngle;
        float closedAngle;
    };

    std::unordered_map<std::string, Part> dynamicParts;

    // RE stuff
    // Please don't modify order, add or remove objects here!
    // If member is unknown and unused, then it should be also commented
    // If member is replaced by something else, then you should point/link to
    // that "something else" using "see ..."

    // CPhysical __parent; // see bullet stuff
    // tHandlingData *m_pVehicleHandling; // see info
    // AutoPilot autoPilot;  // CAutoPilot m_sAutoPilot;
    // uint8_t m_bPrimaryColor; // see colourPrimary
    // uint8_t m_bSecondaryColor; // see colourSecondary
    // char m_bVariationA;
    // char m_bVariationB;
    // int16_t m_wAlarm;
    // char field_418;
    // char field_419;
    // CPed *m_pDriver; // see seatOccupants
    // CPed *m_apPassengers[8]; // see seatOccupants
    // char m_bTotalPassengers; // see seatOccupants
    // char m_bPeopleAttemptingToEnter; // see seatOccupants
    // char m_bSeatsBeingEntered; // see seatOccupants
    // char m_bSeatsTaken; // see seatOccupants
    // char m_bMaxPassengers; // see seatOccupants
    // char field_461;
    // char field_462;
    // char field_463;
    // int field_464;
    // float field_468;
    // float field_472;
    // float field_476;
    // GameObject* CEntity;  // CEntity *m_pCurrentMapObject;
    // CFire *m_pFire;
    // float m_fWheelState;
    // float m_fAccelerationState;
    // float m_fBrakeState;
    // char m_nReference;
    // uint8_t m_bFlags501;
    // uint8_t m_bFlags502;
    // uint8_t m_bFlags503;
    // uint8_t m_bFlags504;
    // char field_505;
    // char field_506;
    // char m_bPacManPickupsCarried;
    // char field_508;
    // char field_509;
    // int16_t m_wRoadblockNode;
    // float m_fHealth; // see health
    // uint8_t m_bGearSimpleCar;
    // char field_517;
    // char field_518;
    // char field_519;
    // float m_fVelocity;
    // int field_524;
    // int m_nTimeDestroyed;
    // int16_t m_nTimeTargetStuckInCar;
    // int16_t m_nTimeUntilExplosion;
    // CEntity *m_pDestoyer;
    // float m_fMapObjectHeightAbove;
    // float m_fMapObjectHeightBelow;
    // CarLock carLock;  // eCarLock m_eCarLock;
    // char m_bLastDamagedWith;
    // RadioStation radioStation;  // eRadioStation m_eRadioStation;
    // char m_bRainAudioCounter;
    // uint8_t m_bRainSamplesCounter;
    // char m_bHornCounter;
    // char field_557;
    // char m_bSirenOrAlarm;
    // char field_559;
    // CStoredCollPoly m_sCollPolys[2];
    // float field_640;
    // VehicleType vehicleType;  // eVehicleType m_nType;

    // end of RE stuff

    VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot,
                  BaseModelInfo* modelinfo, VehicleInfo* info,
                  const glm::u8vec3& prim, const glm::u8vec3& sec);

    ~VehicleObject() override;

    void setPosition(const glm::vec3& pos) override;

    void setRotation(const glm::quat& orientation) override;

    glm::vec3 getCenterOffset() override;

    void updateTransform(const glm::vec3& pos, const glm::quat& rot) override;

    VehicleModelInfo* getVehicle() const {
        return getModelInfo<VehicleModelInfo>();
    }

    Atomic* getHighLOD() const {
        return chassishigh_;
    }
    Atomic* getLowLOD() const {
        return chassislow_;
    }

    Type type() const override {
        return Vehicle;
    }

    bool isWrecked() const;

    void setHealth(float);

    float getHealth() const;

    void setExtraEnabled(size_t extra, bool enabled);

    void setSteeringAngle(float, bool = false);

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

    bool isUpright() const;

    float getVelocity() const;

    void ejectAll();

    GameObject* getOccupant(size_t seat) const;

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

    /**
     * @brief getDriver
     * @return CharacterObject* if there is a driver
     */
    CharacterObject* getDriver() const;

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

    /**
     * @brief isInFront
     * @return a positive distance when the point is in front of the car
     * and a negative distance when the point is behind the car
     */
    float isInFront(const glm::vec3& point);

    /**
     * @brief collectSpecial
     * @return a positive distance when the point is at the right side of the
     * car and a negative distance when the point is at the left side of the car
     */
    float isOnSide(const glm::vec3& point);

    void grantOccupantRewards(CharacterObject* character);

    /**
     * @return The position, and size of the area that must be free for the
     * vehicle to continue.
     */
    std::tuple<glm::vec3, glm::vec3> obstacleCheckVolume() const;

private:
    void setupModel();
    void registerPart(ModelFrame* mf);
    void createObjectHinge(Part* part);
    void destroyObjectHinge(Part* part);
};

#endif
