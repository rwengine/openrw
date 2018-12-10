#include "objects/GameObject.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/Animator.hpp"

GameObject::~GameObject() {
    if (modelinfo_) {
        modelinfo_->removeReference();
    }
}

void GameObject::setPosition(const glm::vec3& pos) {
    _lastPosition = position = pos;
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

glm::mat4 GameObject::getTimeAdjustedTransform(float alpha) const {
    glm::mat4 t{1.0f};
    t = glm::translate(t, glm::mix(_lastPosition, getPosition(), alpha));
    t = t * glm::mat4_cast(glm::slerp(_lastRotation, getRotation(), alpha));
    return t;
}
