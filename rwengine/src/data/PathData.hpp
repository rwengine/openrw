#ifndef _RWENGINE_PATHDATA_HPP_
#define _RWENGINE_PATHDATA_HPP_
#include <stdint.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct PathNode {
    enum NodeType {
        EMPTY = 0,     /// These are ignored
        EXTERNAL = 1,  /// May join with other paths
        INTERNAL = 2   /// Internal to this path
    };

    NodeType type;
    int32_t next;
    glm::vec3 position{};
    float size;
    int other_thing;
    int other_thing2;
};

struct PathData {
    enum PathType { PATH_PED, PATH_CAR };

    PathType type;
    uint16_t ID;
    std::string modelName;
    std::vector<PathNode> nodes;
};

#endif
