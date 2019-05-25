#include "render/ViewFrustum.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 ViewFrustum::projection() const {
    return glm::perspective(fov / aspectRatio, aspectRatio, near, far);
}

void ViewFrustum::update(const glm::mat4 &proj) {
    for (auto i = 0u; i < 6; ++i) {
        float sign = (i % 2 == 0) ? 1.f : -1.f;
        auto r = i / 2;
        planes[i].normal.x = proj[0][3] + proj[0][r] * sign;
        planes[i].normal.y = proj[1][3] + proj[1][r] * sign;
        planes[i].normal.z = proj[2][3] + proj[2][r] * sign;
        planes[i].distance = proj[3][3] + proj[3][r] * sign;

        auto l = glm::length(planes[i].normal);
        planes[i].normal /= l;
        planes[i].distance /= l;
    }
}

bool ViewFrustum::intersects(glm::vec3 center, float radius) const {
    float d;
    bool result = true;

    for (const auto &plane : planes) {
        d = glm::dot(plane.normal, center) + plane.distance;
        if (d < -radius) result = false;
    }

    return result;
}
