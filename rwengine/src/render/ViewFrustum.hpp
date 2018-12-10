#ifndef _RWENGINE_VIEWFRUSTUM_HPP_
#define _RWENGINE_VIEWFRUSTUM_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

class ViewFrustum {
public:
    class ViewPlane {
    public:
	glm::vec3 normal{};
        float distance{};
    };

    float near{};
    float far{};
    float fov{};
    float aspectRatio{};

    ViewPlane planes[6]{};

    ViewFrustum(float near, float far, float fov, float aspect)
        : near(near), far(far), fov(fov), aspectRatio(aspect) {
    }

    glm::mat4 projection();

    void update(const glm::mat4& proj);

    bool intersects(glm::vec3 center, float radius) const;
};

#endif
