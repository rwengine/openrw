#include <boost/test/unit_test.hpp>
#include "test_Globals.hpp"
#include <dynamics/HitTest.hpp>
#include <engine/GameWorld.hpp>
#include <dynamics/CollisionInstance.hpp>
#ifdef _MSC_VER
#pragma warning(disable : 4305 5033)
#endif
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#ifdef _MSC_VER
#pragma warning(default : 4305 5033)
#endif

namespace {

struct HitTestFixture {
    btDefaultCollisionConfiguration collisionConfig;
    btCollisionDispatcher collisionDispatcher;
    btDbvtBroadphase broadphase;
    btGhostPairCallback overlappingPairCallback;
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld dynamicsWorld;
    HitTest hitTest;

    HitTestFixture()
    : collisionDispatcher{&collisionConfig}
    , dynamicsWorld{&collisionDispatcher, &broadphase, &solver, &collisionConfig}
    , hitTest{dynamicsWorld}
    {
        broadphase.getOverlappingPairCache()->setInternalGhostPairCallback(
                &overlappingPairCallback);
    }
};

struct WithSphere : public HitTestFixture {
    btSphereShape shape {0.5f};
    std::unique_ptr<btRigidBody> target;
    GameObject* object {reinterpret_cast<GameObject*>(0xDEADBEEF)};

    WithSphere()
    {
        btDefaultMotionState ms;
        btRigidBody::btRigidBodyConstructionInfo info {1.f, &ms, &shape};
        target = std::make_unique<btRigidBody>(info);
        target->setUserPointer(object);
        dynamicsWorld.addRigidBody(target.get());
    }

    ~WithSphere() {
        dynamicsWorld.removeRigidBody(target.get());
    }
};
}

BOOST_AUTO_TEST_SUITE(HitTestTests)

BOOST_FIXTURE_TEST_CASE(test_creation, HitTestFixture) {
    HitTest test {dynamicsWorld};
}

BOOST_FIXTURE_TEST_CASE(sphereTest_returns_result, WithSphere) {
    const auto result = hitTest.sphereTest({0.f, 0.f, 0.f}, 1.f);
    BOOST_CHECK_EQUAL(result.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(boxTest_returns_result, WithSphere) {
    auto sphereBoundingBoxEdge = glm::vec3{ 0.f, 0.f, shape.getRadius() };
    const auto result = hitTest.boxTest(sphereBoundingBoxEdge, {0.01f, 0.01f, 0.01f});
    BOOST_CHECK_EQUAL(result.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(non_overlapping_test_returns_nothing, WithSphere) {
    auto sphereBoundingBoxEdge = glm::vec3{ shape.getRadius() };
    const auto result = hitTest.boxTest(sphereBoundingBoxEdge * 2.f, {0.01f, 0.01f, 0.01f});
    BOOST_CHECK(result.empty());
}

BOOST_FIXTURE_TEST_CASE(result_contains_body, WithSphere) {
    const auto result = hitTest.sphereTest({0.f, 0.f, 0.f}, 1.f);
    BOOST_ASSERT(result.size() == 1);
    BOOST_CHECK_EQUAL(result[0].body, target.get());
}

BOOST_FIXTURE_TEST_CASE(test_result_contains_object, WithSphere) {
    const auto result = hitTest.sphereTest({0.f, 0.f, 0.f}, 1.f);
    BOOST_ASSERT(result.size() == 1);
    BOOST_CHECK_EQUAL(result[0].object, object);
}

BOOST_AUTO_TEST_SUITE_END()
