#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <glm/gtc/quaternion.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIFP.hpp>
#include <objects/GameObject.hpp>

GameObject::~GameObject() {
    if (animator) {
        delete animator;
    }
    if (skeleton) {
        delete skeleton;
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
    auto hdg = glm::roll(getRotation());
    return hdg / glm::pi<float>() * 180.f;
}

void GameObject::setHeading(float heading) {
    auto hdg = (heading / 180.f) * glm::pi<float>();
    auto quat = glm::normalize(glm::quat(glm::vec3(0.f, 0.f, hdg)));
    setRotation(quat);
}
