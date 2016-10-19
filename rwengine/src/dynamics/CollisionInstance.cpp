#include <dynamics/CollisionInstance.hpp>

#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <objects/GameObject.hpp>

class GameObjectMotionState : public btMotionState {
public:
    GameObjectMotionState(GameObject* object) : m_object(object) {
    }

    virtual void getWorldTransform(btTransform& tform) const {
        auto& position = m_object->getPosition();
        auto& rotation = m_object->getRotation();
        tform.setOrigin(btVector3(position.x, position.y, position.z));
        tform.setRotation(
            btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    }

    virtual void setWorldTransform(const btTransform& tform) {
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
        GameObject* object = static_cast<GameObject*>(m_body->getUserPointer());

        // Remove body from existance.
        object->engine->dynamicsWorld->removeRigidBody(m_body);

        for (btCollisionShape* shape : m_shapes) {
            delete shape;
        }

        delete m_body;
    }
    if (m_vertArray) {
        delete m_vertArray;
    }
    if (m_motionState) {
        delete m_motionState;
    }
}

bool CollisionInstance::createPhysicsBody(GameObject* object,
                                          const std::string& modelName,
                                          DynamicObjectData* dynamics,
                                          VehicleHandlingInfo* handling) {
    auto phyit = object->engine->data->collisions.find(modelName);
    if (phyit != object->engine->data->collisions.end()) {
        btCompoundShape* cmpShape = new btCompoundShape;

        m_motionState = new GameObjectMotionState(object);
        m_shapes.push_back(cmpShape);

        btRigidBody::btRigidBodyConstructionInfo info(0.f, m_motionState,
                                                      cmpShape);

        CollisionModel& physInst = *phyit->second.get();

        float colMin = std::numeric_limits<float>::max(),
              colMax = std::numeric_limits<float>::lowest();

        // Boxes
        for (size_t i = 0; i < physInst.boxes.size(); ++i) {
            auto& box = physInst.boxes[i];
            auto size = (box.max - box.min) / 2.f;
            auto mid = (box.min + box.max) / 2.f;
            btCollisionShape* bshape =
                new btBoxShape(btVector3(size.x, size.y, size.z));
            btTransform t;
            t.setIdentity();
            t.setOrigin(btVector3(mid.x, mid.y, mid.z));
            cmpShape->addChildShape(t, bshape);

            colMin = std::min(colMin, mid.z - size.z);
            colMax = std::max(colMax, mid.z + size.z);

            m_shapes.push_back(bshape);
        }

        // Spheres
        for (size_t i = 0; i < physInst.spheres.size(); ++i) {
            auto& sphere = physInst.spheres[i];
            btCollisionShape* sshape = new btSphereShape(sphere.radius);
            btTransform t;
            t.setIdentity();
            t.setOrigin(
                btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
            cmpShape->addChildShape(t, sshape);

            colMin = std::min(colMin, sphere.center.z - sphere.radius);
            colMax = std::max(colMax, sphere.center.z + sphere.radius);

            m_shapes.push_back(sshape);
        }

        if (physInst.vertices.size() > 0 && physInst.indices.size() >= 3) {
            m_vertArray = new btTriangleIndexVertexArray(
                physInst.indices.size() / 3, (int*)physInst.indices.data(),
                sizeof(uint32_t) * 3, physInst.vertices.size(),
                &(physInst.vertices[0].x), sizeof(glm::vec3));
            btBvhTriangleMeshShape* trishape =
                new btBvhTriangleMeshShape(m_vertArray, false);
            trishape->setMargin(0.05f);
            btTransform t;
            t.setIdentity();
            cmpShape->addChildShape(t, trishape);

            m_shapes.push_back(trishape);
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

        m_body = new btRigidBody(info);
        m_body->setUserPointer(object);
        object->engine->dynamicsWorld->addRigidBody(m_body);

        if (dynamics && dynamics->uprootForce > 0.f) {
            m_body->setCollisionFlags(
                m_body->getCollisionFlags() |
                btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        }
    } else {
        return false;
    }

    return true;
}

void CollisionInstance::changeMass(float newMass) {
    GameObject* object = static_cast<GameObject*>(m_body->getUserPointer());
    auto& dynamicsWorld = object->engine->dynamicsWorld;
    dynamicsWorld->removeRigidBody(m_body);
    btVector3 inert;
    m_body->getCollisionShape()->calculateLocalInertia(newMass, inert);
    m_body->setMassProps(newMass, inert);
    dynamicsWorld->addRigidBody(m_body);
}
