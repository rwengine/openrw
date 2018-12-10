#ifndef _RWENGINE_COLLISIONMODEL_HPP_
#define _RWENGINE_COLLISIONMODEL_HPP_

#include <glm/vec3.hpp>

#include <cstdint>
#include <string>
#include <vector>

/**
 * @class CollisionModel
 * Collision shapes data container.
 */
struct CollisionModel {
    struct Surface {
        uint8_t material;
        uint8_t flag;
        uint8_t brightness;
        uint8_t light;
    };

    /// @todo give shapes surface data
    struct Sphere {
        glm::vec3 center{};
        float radius;
        Surface surface;
    };

    struct Box {
        glm::vec3 min{};
        glm::vec3 max{};
        Surface surface;
    };

    struct Triangle {
        uint32_t tri[3];
        Surface surface;
    };

    /// Model name
    std::string name;
    uint16_t modelid;

    Sphere boundingSphere;
    Box boundingBox;

    std::vector<Sphere> spheres;
    std::vector<Box> boxes;
    std::vector<glm::vec3> vertices;
    std::vector<Triangle> faces;
};

#endif
