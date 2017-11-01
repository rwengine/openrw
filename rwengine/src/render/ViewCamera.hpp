#ifndef _RWENGINE_VIEWCAMERA_HPP_
#define _RWENGINE_VIEWCAMERA_HPP_
#include <glm/gtc/quaternion.hpp>

#include "render/ViewFrustum.hpp"

class ViewCamera {
public:
    ViewFrustum frustum;

    glm::vec3 position;
    glm::quat rotation;

    ViewCamera(const glm::vec3& pos = {}, const glm::quat& rot = {})
        : frustum({0.1f, 5000.f, glm::radians(45.f), 1.f})
        , position(pos)
        , rotation(rot) {
    }

    glm::mat4 getView() {
        auto up = rotation * glm::vec3(0.f, 0.f, 1.f);
        return glm::lookAt(position,
                           position + rotation * glm::vec3(1.f, 0.f, 0.f), up);
    }
};

#endif
