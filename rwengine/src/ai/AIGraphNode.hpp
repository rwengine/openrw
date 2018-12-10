#ifndef _RWENGINE_AIGRAPHNODE_HPP_
#define _RWENGINE_AIGRAPHNODE_HPP_

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace ai {

enum class NodeType { Vehicle, Pedestrian };

struct AIGraphNode {
    enum {
        None = 0,
        CrossesRoad =
            1  /// No documentation for other flags yet, but this is mentioned.
    };

    NodeType type;
    glm::vec3 position{};
    float size;
    int leftLanes;
    int rightLanes;
    bool external;
    uint8_t flags;

    int32_t nextIndex;

    bool disabled;

    std::vector<AIGraphNode*> connections;
};

}  // namespace ai

#endif
