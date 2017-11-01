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
class InstanceObject : public GameObject {
    float health;
    bool visible = true;

    /**
     * The Atomic instance for this object
     */
    AtomicPtr atomic_;

public:
    glm::vec3 scale;
    std::unique_ptr<CollisionInstance> body;
    std::shared_ptr<DynamicObjectData> dynamics;
    bool _enablePhysics;

    InstanceObject(GameWorld* engine, const glm::vec3& pos,
                   const glm::quat& rot, const glm::vec3& scale,
                   BaseModelInfo* modelinfo,
                   std::shared_ptr<DynamicObjectData> dyn);
    ~InstanceObject();

    Type type() const override {
        return Instance;
    }

    const AtomicPtr& getAtomic() const {
        return atomic_;
    }

    void tick(float dt) override;

    void changeModel(BaseModelInfo* incoming);

    virtual void setPosition(const glm::vec3& pos) override;
    virtual void setRotation(const glm::quat& r) override;

    virtual bool takeDamage(const DamageInfo& damage) override;

    void setSolid(bool solid);

    void setVisible(bool visible) {
        this->visible = visible;
    }
    float getVisible() const {
        return visible;
    }

    float getHealth() const {
        return health;
    }

    void updateTransform(const glm::vec3& pos, const glm::quat& rot) override;
};

#endif
