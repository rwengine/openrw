#ifndef _RWENGINE_INSTANCEOBJECT_HPP_
#define _RWENGINE_INSTANCEOBJECT_HPP_
#include <memory>

#include <glm/glm.hpp>

#include <rw/forward.hpp>

#include <objects/GameObject.hpp>

class BaseModelInfo;
class CollisionInstance;
class GameWorld;

/**
 * @struct InstanceObject
 *  A simple object instance
 */
class InstanceObject final : public GameObject {
    float health = 100.f;
    bool visible =true;
    bool floating = false;
    bool static_ = false;
    bool usePhysics = false;
    int changeAtomic = -1;

    /**
     * The Atomic instance for this object
     */
    AtomicPtr atomic_;

public:
    glm::vec3 scale;
    std::unique_ptr<CollisionInstance> body;
    DynamicObjectData* dynamics;

    InstanceObject(GameWorld* engine, const glm::vec3& pos,
                   const glm::quat& rot, const glm::vec3& scale,
                   BaseModelInfo* modelinfo,
                   DynamicObjectData* dyn);
    ~InstanceObject() override;

    Type type() const override {
        return Instance;
    }

    const AtomicPtr& getAtomic() const {
        return atomic_;
    }

    void tick(float dt) override;

    void tickPhysics(float dt);

    void changeModel(BaseModelInfo* incoming, int atomicNumber = 0);

    void setPosition(const glm::vec3& pos) override;

    void setRotation(const glm::quat& r) override;

    bool takeDamage(const DamageInfo& damage) override;

    void setSolid(bool solid);

    void setStatic(bool s);

    bool isStatic() const {
        return static_;
    }

    void setVisible(bool v) {
        visible = v;
    }

    bool isVisible() const {
        return visible;
    }

    void setFloating(bool f) {
        floating = f;
    }

    bool isFloating() const {
        return floating;
    }

    float getHealth() const {
        return health;
    }

    void updateTransform(const glm::vec3& pos, const glm::quat& rot) override;
};

#endif
