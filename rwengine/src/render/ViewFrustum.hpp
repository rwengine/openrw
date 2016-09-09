#ifndef _VIEWFRUSTUM_HPP_
#define _VIEWFRUSTUM_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

class ViewFrustum {
public:
    class ViewPlane {
    public:
        glm::vec3 normal;
        float distance;
    };

    float near;
    float far;
    float fov;
    float aspectRatio;

    ViewPlane planes[6];

    ViewFrustum(float near, float far, float fov, float aspect)
        : near(near), far(far), fov(fov), aspectRatio(aspect) {
    }

    glm::mat4 projection() {
        return glm::perspective(fov / aspectRatio, aspectRatio, near, far);
    }

    void update(const glm::mat4& proj) {
        for (size_t i = 0; i < 6; ++i) {
            float sign = (i % 2 == 0) ? 1.f : -1.f;
            int r = i / 2;
            planes[i].normal.x = proj[0][3] + proj[0][r] * sign;
            planes[i].normal.y = proj[1][3] + proj[1][r] * sign;
            planes[i].normal.z = proj[2][3] + proj[2][r] * sign;
            planes[i].distance = proj[3][3] + proj[3][r] * sign;

            auto l = glm::length(planes[i].normal);
            planes[i].normal /= l;
            planes[i].distance /= l;
        }
    }

    bool intersects(glm::vec3 center, float radius) const {
        float d;
        bool result = true;

        for (size_t i = 0; i < 6; ++i) {
            d = glm::dot(planes[i].normal, center) + planes[i].distance;
            if (d < -radius) result = false;
        }

        return result;
    }
};

#endif
