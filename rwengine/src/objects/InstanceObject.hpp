#pragma once
#ifndef _OBJECTINSTANCE_HPP_
#define _OBJECTINSTANCE_HPP_
#include <btBulletDynamicsCommon.h>
#include <objects/GameObject.hpp>

class CollisionInstance;

/**
 * @struct InstanceObject
 *  A simple object instance
 */
class InstanceObject : public GameObject {
    float health;
    bool visible = true;

public:
    glm::vec3 scale;
    CollisionInstance* body;
    InstanceObject* LODinstance;
    std::shared_ptr<DynamicObjectData> dynamics;
    bool _enablePhysics;

    InstanceObject(GameWorld* engine, const glm::vec3& pos,
                   const glm::quat& rot, const glm::vec3& scale,
                   BaseModelInfo* modelinfo, InstanceObject* lod,
                   std::shared_ptr<DynamicObjectData> dyn);
    ~InstanceObject();

    Type type() {
        return Instance;
    }

    void tick(float dt);

    void changeModel(BaseModelInfo* incoming);

    virtual void setRotation(const glm::quat& r);

    virtual bool takeDamage(const DamageInfo& damage);

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
};

#endif
