#ifndef _RWENGINE_COLLISIONINSTANCE_HPP_
#define _RWENGINE_COLLISIONINSTANCE_HPP_

#include <memory>
#include <vector>

class btCollisionShape;
class btCompoundShape;
class btMotionState;
class btRigidBody;
class btTriangleIndexVertexArray;
struct CollisionModel;

class GameObject;
struct DynamicObjectData;
struct VehicleHandlingInfo;

/**
 * @brief CollisionInstance stores bullet body information
 */
class CollisionInstance {
public:
    CollisionInstance() = default;

    ~CollisionInstance();

    bool createPhysicsBody(GameObject* object, CollisionModel* collision,
                           DynamicObjectData* dynamics = nullptr,
                           VehicleHandlingInfo* handling = nullptr);

    btRigidBody* getBulletBody() const {
        return m_body.get();
    }

    float getBoundingHeight() const {
        return m_collisionHeight;
    }

    void changeMass(float newMass);

private:
    std::unique_ptr<btRigidBody> m_body;

    std::unique_ptr<btCompoundShape> cmpShape;
    std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
    std::unique_ptr<btTriangleIndexVertexArray> m_vertArray;

    std::unique_ptr<btMotionState> m_motionState;

    float m_collisionHeight{0.f};
};

#endif
