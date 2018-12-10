#ifndef _RWENGINE_AIGRAPH_HPP_
#define _RWENGINE_AIGRAPH_HPP_

#include <glm/gtc/quaternion.hpp>

#include <rw/types.hpp>

#include <array>
#include <vector>

struct PathData;

namespace ai {

enum class NodeType;
struct AIGraphNode;

class AIGraph {
public:
    ~AIGraph() = default;

    std::vector<std::unique_ptr<AIGraphNode>> nodes;

    /**
     * List of external nodes, which are links between each
     * Instance's paths and where new pedestrians and vehicles
     * are spawned
     */
    std::vector<AIGraphNode*> externalNodes;

    /**
     * Stores the external AI Grid Nodes organised by world grid cell
     */
    std::array<std::vector<AIGraphNode*>, WORLD_GRID_CELLS> gridNodes;

    void createPathNodes(const glm::vec3& position, const glm::quat& rotation,
                         PathData& path);

    void gatherExternalNodesNear(const glm::vec3& center, const float radius,
                                 std::vector<AIGraphNode*>& nodes, NodeType type);
};

} // ai

#endif
