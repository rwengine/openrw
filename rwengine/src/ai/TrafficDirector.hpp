#ifndef _RWENGINE_TRAFFICDIRECTOR_HPP_
#define _RWENGINE_TRAFFICDIRECTOR_HPP_

#include "AIGraphNode.hpp"

#include <vector>

class AIGraph;
class GameObject;
class GameWorld;
class ViewCamera;

class TrafficDirector {
public:
    TrafficDirector(AIGraph* graph, GameWorld* world);

    std::vector<AIGraphNode*> findAvailableNodes(AIGraphNode::NodeType type,
                                                 const ViewCamera& camera,
                                                 float radius);

    void setDensity(AIGraphNode::NodeType type, float density);

    /**
     * Creates new traffic at available locations.
     * @param camera The camera to spawn around
     * @param radius the maximum distance to spawn in
     * @param max The maximum number of traffic to create.
     */
    std::vector<GameObject*> populateNearby(const ViewCamera& camera,
                                            float radius, int maxSpawn = -1);

    /**
     * Sets the maximum number of pedestrians and cars in the traffic system
     */
    void setPopulationLimits(int maxPeds, int maxCars);

private:
    AIGraph* graph;
    GameWorld* world;
    float pedDensity;
    float carDensity;
    int maximumPedestrians;
    int maximumCars;
};

#endif
