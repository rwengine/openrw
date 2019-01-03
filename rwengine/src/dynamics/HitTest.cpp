#include "HitTest.hpp"
#include <objects/GameObject.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4305 5033)
#endif
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#ifdef _MSC_VER
#pragma warning(default : 4305 5033)
#endif

namespace {

HitTest::TestResult HitTestWorld(btDiscreteDynamicsWorld& world, btPairCachingGhostObject& tester)
{
    world.addCollisionObject(&tester);

    HitTest::TestResult result;
    result.reserve(static_cast<unsigned long>(tester.getNumOverlappingObjects()));

    for (auto i = 0; i < tester.getNumOverlappingObjects(); ++i)
    {
        auto overlapping = tester.getOverlappingObject(i);

        HitTest::Hit hit{};
        hit.body = overlapping;

        if (auto object = static_cast<GameObject*>(overlapping->getUserPointer()))
        {
            hit.object = object;
        }

        result.push_back(hit);
    }

    world.removeCollisionObject(&tester);

    return result;
}

HitTest::TestResult HitTestWithShape(btDiscreteDynamicsWorld& world, btCollisionShape& shape,
        const glm::vec3& center, const glm::quat& rotation) {
    btPairCachingGhostObject ghost{};
    btTransform xform{};
    xform.setOrigin({center.x, center.y, center.z});
    xform.setRotation({rotation.x, rotation.y, rotation.z, rotation.w});
    ghost.setWorldTransform(xform);
    ghost.setCollisionShape(&shape);
    return HitTestWorld(world, ghost);
}

} // namespace

HitTest::TestResult HitTest::sphereTest(const glm::vec3& center, float radius) {
    btSphereShape sphere {radius};
    return HitTestWithShape(_world, sphere, center, {});
}

HitTest::TestResult HitTest::boxTest(const glm::vec3 &center, const glm::vec3 &size, const glm::quat& rotation) {
    btBoxShape box {{size.x, size.y, size.z}};
    return HitTestWithShape(_world, box, center, rotation);
}
