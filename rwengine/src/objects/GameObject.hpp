#ifndef _RWENGINE_GAMEOBJECT_HPP_
#define _RWENGINE_GAMEOBJECT_HPP_

#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <rw/debug.hpp>
#include <rw/forward.hpp>

#include <data/ModelData.hpp>
#include <engine/Animator.hpp>
#include <objects/ObjectTypes.hpp>

class GameWorld;

/**
 * @brief Base data and interface for all world "objects" like vehicles, peds.
 *
 * Contains handle to the world, and other useful properties like water level
 * tracking used to make tunnels work.
 */
class GameObject {
    glm::vec3 _lastPosition;
    glm::quat _lastRotation;
    GameObjectID objectID = 0;

    BaseModelInfo* modelinfo_;

    /**
     * Model used for rendering
     */
    ClumpPtr model_ = nullptr;

protected:
    void changeModelInfo(BaseModelInfo* next) {
        modelinfo_ = next;
    }

public:
    glm::vec3 position;
    glm::quat rotation;

    GameWorld* engine = nullptr;

    std::unique_ptr<Animator> animator;  /// Object's animator.

    bool inWater = false;

    /**
     * @brief stores the height of water at the last tick
     */
    float _lastHeight = std::numeric_limits<float>::max();

    /**
     * Should object be rendered?
     */
    bool visible = true;

    GameObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot,
               BaseModelInfo* modelinfo)
        : _lastPosition(pos)
        , _lastRotation(rot)
        , modelinfo_(modelinfo)
        , position(pos)
        , rotation(rot)
        , engine(engine) {
        if (modelinfo_) {
            modelinfo_->addReference();
        }
    }

    virtual ~GameObject();

    GameObjectID getGameObjectID() const {
        return objectID;
    }
    /**
     * Do not call this, use GameWorld::insertObject
     */
    void setGameObjectID(GameObjectID id) {
        objectID = id;
    }

    int getScriptObjectID() const {
        return getGameObjectID();
    }

    template <class T>
    T* getModelInfo() const {
        return static_cast<T*>(modelinfo_);
    }

    /**
     * @return The model used in rendering
     */
    ClumpPtr getModel() const {
        return model_;
    }

    /**
     * Changes the current model, used for re-dressing chars
     */
    void setModel(const ClumpPtr& model) {
        model_ = model;
    }

    /**
     * @brief Enumeration of possible object types.
     */
    enum Type {
        Instance,
        Character,
        Vehicle,
        Pickup,
        Projectile,
        Cutscene,
        Unknown
    };

    /**
     * @brief determines what type of object this is.
     * @return one of Type
     */
    virtual Type type() const {
        return Unknown;
    }

    virtual void setPosition(const glm::vec3& pos);

    const glm::vec3& getPosition() const {
        return position;
    }
    const glm::vec3& getLastPosition() const {
        return _lastPosition;
    }

    const glm::quat& getRotation() const {
        return rotation;
    }
    virtual void setRotation(const glm::quat& orientation);

    float getHeading() const;
    /**
     * @brief setHeading Rotates the object to face heading, in degrees.
     */
    void setHeading(float heading);

    /**
     * @brief getCenterOffset Returns the offset from center of mass to base of model
     * This function should be overwritten by a derived class
     */
    virtual glm::vec3 getCenterOffset() {
        return glm::vec3(0.f, 0.f, 1.f);
    }

    /**
     * @brief applyOffset Applies the offset from getCenterOffset to the object
     */
    void applyOffset() {
         setPosition(getPosition() + getCenterOffset());
    }

    struct DamageInfo {
        enum class DamageType {
            Explosion, Burning, Bullet, Physics, Melee
        };

        /**
         * World position of damage
         */
        glm::vec3 damageLocation{};

        /**
         * World position of the source (used for direction)
         */
        glm::vec3 damageSource{};

        /**
         * Magnitude of destruction
         */
        float hitpoints;

        /**
         * Type of the damage
         */
        DamageType type;

        /**
         * Physics impulse.
         */
        float impulse;

        DamageInfo(DamageType type, const glm::vec3 &location,
                   const glm::vec3 &source, float damage, float impulse = 0.f)
            : damageLocation(location), damageSource(source), hitpoints(damage),
              type(type), impulse(impulse) {}
    };

    virtual bool takeDamage(const DamageInfo& damage) {
        RW_UNUSED(damage);
        return false;
    }

    virtual bool isAnimationFixed() const {
        return true;
    }

    virtual bool isInWater() const {
        return inWater;
    }

    virtual void tick(float dt) = 0;

    /**
     * @brief Function used to modify the last transform
     * @param newPos
     */
    void _updateLastTransform() {
        _lastPosition = getPosition();
        _lastRotation = getRotation();
    }

    glm::mat4 getTimeAdjustedTransform(float alpha) const {
        glm::mat4 t{1.0f};
        t = glm::translate(t, glm::mix(_lastPosition, getPosition(), alpha));
        t = t * glm::mat4_cast(glm::slerp(_lastRotation, getRotation(), alpha));
        return t;
    }

    enum ObjectLifetime {
        /// lifetime has not been set
        UnknownLifetime,
        /// Generic background pedestrians
        TrafficLifetime,
        /// Part of a mission
        MissionLifetime,
        /// Is owned by the player (or is the player)
        PlayerLifetime
    };

    void setLifetime(ObjectLifetime ol) {
        lifetime = ol;
    }
    ObjectLifetime getLifetime() const {
        return lifetime;
    }

    virtual void updateTransform(const glm::vec3& pos, const glm::quat& rot) {
        _lastPosition = position;
        _lastRotation = rotation;
        position = pos;
        rotation = rot;
    }

private:
    ObjectLifetime lifetime = GameObject::UnknownLifetime;
};

class ClumpObject {
    ClumpPtr clump_;

protected:
    void setClump(const ClumpPtr& ptr) {
        clump_ = ptr;
    }

public:
    const ClumpPtr& getClump() const {
        return clump_;
    }
};

#endif  // __GAMEOBJECTS_HPP__
