#ifndef _RWENGINE_AIGRAPHNODE_HPP_
#define _RWENGINE_AIGRAPHNODE_HPP_
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct AIGraphNode {
    enum NodeType { Vehicle, Pedestrian };

    enum {
        None = 0,
        CrossesRoad =
            1  /// No documentation for other flags yet, but this is mentioned.
    };

    NodeType type;
    glm::vec3 position;
    float size;
    int other_thing;
    int other_thing2;
    bool external;
    uint8_t flags;

    int32_t nextIndex;

    bool disabled;

    std::vector<AIGraphNode*> connections;
};

#endif
