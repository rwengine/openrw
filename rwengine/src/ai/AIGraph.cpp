#include "ai/AIGraph.hpp"

#include <algorithm>
#include <cstddef>

#include <glm/gtx/norm.hpp>

#include "ai/AIGraphNode.hpp"
#include "data/PathData.hpp"

#include <rw/debug.hpp>
#include <rw/types.hpp>

void AIGraph::createPathNodes(const glm::vec3& position,
                              const glm::quat& rotation, PathData& path) {
    auto startIndex = static_cast<std::uint32_t>(nodes.size());
    std::vector<AIGraphNode*> pathNodes;
    pathNodes.reserve(path.nodes.size());
    nodes.reserve(path.nodes.size());

    for (auto n = 0u; n < path.nodes.size(); ++n) {
        bool external = false;
        auto& node = path.nodes[n];
        glm::vec3 nodePosition = position + (rotation * node.position);

        if (node.type == PathNode::EXTERNAL) {
            for (auto& realNode : externalNodes) {
                auto d = glm::distance2(realNode->position, nodePosition);
                if (d < 1.f) {
                    pathNodes.push_back(realNode);
                    external = true;
                    break;
                }
            }
        }
        if (!external) {
            auto ainode = std::make_unique<AIGraphNode>();
            auto ptr = ainode.get();

            ainode->type =
                (path.type == PathData::PATH_PED ? AIGraphNode::Pedestrian
                                                 : AIGraphNode::Vehicle);
            ainode->nextIndex = node.next >= 0 ? startIndex + node.next : -1;
            ainode->flags = AIGraphNode::None;
            ainode->size = node.size;
            ainode->leftLanes = node.leftLanes;
            ainode->rightLanes = node.rightLanes;
            ainode->position = nodePosition;
            ainode->external = node.type == PathNode::EXTERNAL;
            ainode->disabled = false;

            pathNodes.push_back(ptr);
            nodes.push_back(std::move(ainode));

            if (ptr->external) {
                externalNodes.push_back(ptr);

                // Determine which grid cell this node falls into
                float lowerCoord = -(WORLD_GRID_SIZE) / 2.f;
                auto gridrel = glm::vec2(ptr->position) -
                               glm::vec2(lowerCoord, lowerCoord);
                auto gridcoord =
                    glm::floor(gridrel / glm::vec2(WORLD_CELL_SIZE));
                if (gridcoord.x < 0 || gridcoord.y < 0 ||
                    gridcoord.x >= WORLD_GRID_WIDTH ||
                    gridcoord.y >= WORLD_GRID_WIDTH) {
                    RW_MESSAGE("Warning: Node outside of grid at coord "
                               << gridcoord.x << " " << gridcoord.y);
                }
                auto index = static_cast<std::size_t>(
                    (gridcoord.x * WORLD_GRID_WIDTH) + gridcoord.y);
                gridNodes[index].push_back(ptr);
            }
        }
    }

    for (auto pn = 0u; pn < path.nodes.size(); ++pn) {
        if (path.nodes[pn].next >= 0 &&
            static_cast<unsigned>(path.nodes[pn].next) < pathNodes.size()) {
            auto node = pathNodes[pn];
            auto next = pathNodes[path.nodes[pn].next];

            node->connections.push_back(next);
            next->connections.push_back(node);
        }
    }
}

glm::ivec2 worldToGrid(const glm::vec2& world) {
    static const float lowerCoord = -(WORLD_GRID_SIZE) / 2.f;
    return glm::ivec2((world - glm::vec2(lowerCoord)) /
                      glm::vec2(WORLD_CELL_SIZE));
}

void AIGraph::gatherExternalNodesNear(const glm::vec3& center,
                                      const float radius,
                                      std::vector<AIGraphNode*>& nodes,
                                      AIGraphNode::NodeType type) {
    // the bounds end up covering more than might fit
    auto planecoords = glm::vec2(center);
    auto minWorld = planecoords - glm::vec2(radius);
    auto maxWorld = planecoords + glm::vec2(radius);
    auto minGrid = worldToGrid(minWorld);
    auto maxGrid = worldToGrid(maxWorld);

    for (int x = minGrid.x; x <= maxGrid.x; ++x) {
        for (int y = minGrid.y; y <= maxGrid.y; ++y) {
            int i = (x * WORLD_GRID_WIDTH) + y;
            if (i < 0 || i >= static_cast<int>(gridNodes.size())) {
                continue;
            }
            auto& external = gridNodes[i];
            copy_if(external.begin(), external.end(), back_inserter(nodes),
                    [&center, &radius, &type](const auto node) {
                        return node->type == type &&
                               glm::distance2(center, node->position) <
                                   radius * radius;
                    });
        }
    }
}
