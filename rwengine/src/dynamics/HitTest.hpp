#ifndef _RWENGINE_HITTEST_HPP_
#define _RWENGINE_HITTEST_HPP_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <memory>

class btDiscreteDynamicsWorld;
class btCollisionObject;
class GameObject;

/**
 * Utility for performing collision tests against the world.
 */
class HitTest {
public:
    struct Hit {
        btCollisionObject* body;
        GameObject* object;
    };
    using TestResult = std::vector<Hit>;

    explicit HitTest(btDiscreteDynamicsWorld& world)
        : _world(world)
    {}

    ~HitTest() = default;

    TestResult sphereTest(const glm::vec3& center, float radius);
    TestResult boxTest(const glm::vec3& center, const glm::vec3& size, const glm::quat& rotation = {1.f, 0.f, 0.f, 0.f});

private:
    btDiscreteDynamicsWorld& _world;
};


#endif
