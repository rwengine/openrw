#include "dynamics/CollisionInstance.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "data/CollisionModel.hpp"
#include "data/ModelData.hpp"
#include "engine/GameWorld.hpp"
#include "objects/GameObject.hpp"
#include "objects/VehicleInfo.hpp"

class GameObjectMotionState : public btMotionState {
public:
    GameObjectMotionState(GameObject* object) : m_object(object) {
    }

    void getWorldTransform(btTransform& tform) const override {
        auto& position = m_object->getPosition();
        auto& rotation = m_object->getRotation();
        tform.setOrigin(btVector3(position.x, position.y, position.z));
        tform.setRotation(
            btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    }

    void setWorldTransform(const btTransform& tform) override {
        auto& o = tform.getOrigin();
        auto r = tform.getRotation();
        glm::vec3 position(o.x(), o.y(), o.z());
        glm::quat rotation(r.w(), r.x(), r.y(), r.z());
        m_object->updateTransform(position, rotation);
    }

private:
    GameObject* m_object;
};

CollisionInstance::~CollisionInstance() {
    if (m_body) {
        auto object = static_cast<GameObject*>(m_body->getUserPointer());
        object->engine->dynamicsWorld->removeRigidBody(m_body.get());
    }
}

bool CollisionInstance::createPhysicsBody(GameObject* object,
                                          CollisionModel* collision,
                                          DynamicObjectData* dynamics,
                                          VehicleHandlingInfo* handling) {
    cmpShape = std::make_unique<btCompoundShape>();

    m_motionState = std::make_unique<GameObjectMotionState>(object);
    btRigidBody::btRigidBodyConstructionInfo info(0.f, m_motionState.get(),
                                                  cmpShape.get());

    float colMin = std::numeric_limits<float>::max(),
          colMax = std::numeric_limits<float>::lowest();

    btTransform t;
    t.setIdentity();

    // Boxes
    for (const auto &box : collision->boxes) {
        auto size = (box.max - box.min) / 2.f;
        auto mid = (box.min + box.max) / 2.f;
        auto bshape = std::make_unique<btBoxShape>(
            btVector3(size.x, size.y, size.z));
        t.setOrigin(btVector3(mid.x, mid.y, mid.z));
        cmpShape->addChildShape(t, bshape.get());

        colMin = std::min(colMin, mid.z - size.z);
        colMax = std::max(colMax, mid.z + size.z);

        m_shapes.push_back(std::move(bshape));
    }

    // Spheres
    for (const auto &sphere : collision->spheres) {
        auto sshape = std::make_unique<btSphereShape>(sphere.radius);
        t.setOrigin(
            btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
        cmpShape->addChildShape(t, sshape.get());

        colMin = std::min(colMin, sphere.center.z - sphere.radius);
        colMax = std::max(colMax, sphere.center.z + sphere.radius);

        m_shapes.push_back(std::move(sshape));
    }

    t.setIdentity();
    auto& verts = collision->vertices;
    auto& faces = collision->faces;
    if (!verts.empty() && !faces.empty()) {
        m_vertArray = std::make_unique<btTriangleIndexVertexArray>(
            static_cast<int>(faces.size()),
            reinterpret_cast<int*>(faces.data()),
            static_cast<int>(sizeof(CollisionModel::Triangle)),
            static_cast<int>(verts.size()),
            reinterpret_cast<float*>(verts.data()),
            static_cast<int>(sizeof(glm::vec3)));
        auto trishape =
            std::make_unique<btBvhTriangleMeshShape>(m_vertArray.get(), false);
        trishape->setMargin(0.05f);
        cmpShape->addChildShape(t, trishape.get());

        m_shapes.push_back(std::move(trishape));
    }

    m_collisionHeight = colMax - colMin;

    if (dynamics) {
        if (dynamics->uprootForce > 0.f) {
            info.m_mass = 0.f;
        } else {
            btVector3 inert;
            cmpShape->calculateLocalInertia(dynamics->mass, inert);
            info.m_mass = dynamics->mass;
            info.m_localInertia = inert;
        }
    } else if (handling) {
        btVector3 inert;
        cmpShape->calculateLocalInertia(handling->mass, inert);
        info.m_mass = handling->mass;
        info.m_localInertia = inert;
    }

    m_body = std::make_unique<btRigidBody>(info);
    m_body->setUserPointer(object);
    object->engine->dynamicsWorld->addRigidBody(m_body.get());

    return true;
}

void CollisionInstance::changeMass(float newMass) {
    auto object = static_cast<GameObject*>(m_body->getUserPointer());
    auto& dynamicsWorld = object->engine->dynamicsWorld;
    dynamicsWorld->removeRigidBody(m_body.get());
    btVector3 inert;
    m_body->getCollisionShape()->calculateLocalInertia(newMass, inert);
    m_body->setMassProps(newMass, inert);
    dynamicsWorld->addRigidBody(m_body.get());
}
