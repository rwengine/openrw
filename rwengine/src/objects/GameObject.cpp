#include "objects/GameObject.hpp"

#include <data/Clump.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/Animator.hpp"

const AtomicPtr GameObject::NullAtomic;
const ClumpPtr GameObject::NullClump;

GameObject::~GameObject() {
    if (modelinfo_) {
        modelinfo_->removeReference();
    }
}

void GameObject::setPosition(const glm::vec3& pos) {
    position = pos;
}

void GameObject::setRotation(const glm::quat& orientation) {
    rotation = orientation;
}

float GameObject::getHeading() const {
    auto hdg = glm::roll(getRotation());
    return hdg / glm::pi<float>() * 180.f;
}

void GameObject::setHeading(float heading) {
    auto hdg = (heading / 180.f) * glm::pi<float>();
    auto quat = glm::normalize(glm::quat(glm::vec3(0.f, 0.f, hdg)));
    setRotation(quat);
}

void GameObject::updateTransform(const glm::vec3& pos, const glm::quat& rot) {
    position = pos;
    rotation = rot;

    const auto& clump = getClump();
    const auto& atomic = getAtomic();
    if (clump) {
        clump->getFrame()->setRotation(glm::mat3_cast(rot));
        clump->getFrame()->setTranslation(pos);
    }
    if (atomic) {
        atomic->getFrame()->setRotation(glm::mat3_cast(rot));
        atomic->getFrame()->setTranslation(pos);
    }
}
