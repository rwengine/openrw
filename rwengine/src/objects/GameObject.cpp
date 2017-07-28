#include <engine/Animator.hpp>
#include <glm/gtc/quaternion.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIFP.hpp>
#include <objects/GameObject.hpp>

GameObject::~GameObject() {
    if (animator) {
        delete animator;
    }

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
    return glm::degrees(glm::roll(getRotation()));
}

void GameObject::setHeading(float heading) {
    auto quat = glm::normalize(glm::quat(glm::vec3(0.f, 0.f, glm::radians(heading))));
    setRotation(quat);
}
