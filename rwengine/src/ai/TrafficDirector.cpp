#include "ai/TrafficDirector.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <glm/glm.hpp>

#include "ai/AIGraph.hpp"
#include "ai/AIGraphNode.hpp"
#include "ai/CharacterController.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/VehicleObject.hpp"
#include "render/ViewCamera.hpp"

#include <glm/gtx/norm.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

TrafficDirector::TrafficDirector(AIGraph* g, GameWorld* w)
    : graph(g)
    , world(w) {
}

std::vector<AIGraphNode*> TrafficDirector::findAvailableNodes(
    AIGraphNode::NodeType type, const ViewCamera& camera, float radius) {
    std::vector<AIGraphNode*> available;
    available.reserve(20);

    graph->gatherExternalNodesNear(camera.position, radius, available);

    float density = type == AIGraphNode::Vehicle ? carDensity : pedDensity;
    float minDist = (10.f / density) * (10.f / density);
    float halfRadius2 = std::pow(radius / 2.f, 2.f);

    // Check if any of the nearby nodes are blocked by a pedestrian standing on
    // it
    // or because it's inside the view frustum
    for (auto it = available.begin(); it != available.end();) {
        bool blocked = false;
        float dist2 = glm::distance2(camera.position, (*it)->position);

        for (auto& obj : world->pedestrianPool.objects) {
            if (glm::distance2((*it)->position, obj.second->getPosition()) <=
                minDist) {
                blocked = true;
                break;
            }
        }

        // Check that we're not going to spawn something right where the player
        // is looking
        if (dist2 <= halfRadius2 &&
            camera.frustum.intersects((*it)->position, 1.f)) {
            blocked = true;
        }

        if (blocked) {
            it = available.erase(it);
        } else {
            it++;
        }
    }

    return available;
}

void TrafficDirector::setDensity(AIGraphNode::NodeType type, float density) {
    switch (type) {
        case AIGraphNode::Vehicle:
            carDensity = density;
            break;
        case AIGraphNode::Pedestrian:
            pedDensity = density;
            break;
    }
}

std::vector<GameObject*> TrafficDirector::populateNearby(
    const ViewCamera& camera, float radius, int maxSpawn) {
    int availablePeds =
        maximumPedestrians - world->pedestrianPool.objects.size();

    std::vector<GameObject*> created;

    /// @todo Check how "in player view" should be determined.

    // Don't check the frustum for things more than 1/2 of the radius away
    // so that things will spawn as you drive towards them
    float halfRadius2 = std::pow(radius / 2.f, 2.f);

    // Spawn vehicles at vehicle generators
    auto camera2D = glm::vec2(camera.position);
    for (auto& gen : world->state->vehicleGenerators) {
        /// @todo verify how vehicle generator proximity is determined
        auto gen2D = glm::vec2(gen.position);
        float dist2 = glm::distance2(camera2D, gen2D);
        if (dist2 < radius * radius) {
            auto position = gen.position;
            // Check that the on-ground position is not in view
            if (gen.position.z < -90.f) {
                position = world->getGroundAtPosition(position);
            }

            if (dist2 <= halfRadius2 &&
                camera.frustum.intersects(position, 1.f)) {
                if (!gen.alwaysSpawn) {
                    // Don't spawn in the view frustum unless we're forced to
                    continue;
                }
            }
            auto spawned = world->tryToSpawnVehicle(gen);
            if (spawned) {
                created.push_back(spawned);
            }
        }
    }

    auto type = AIGraphNode::Pedestrian;
    auto available = findAvailableNodes(type, camera, radius);

    if (availablePeds <= 0) {
        // We have already reached the limit of spawned traffic
        return {};
    }

    /// Hardcoded cop Pedestrian
    std::vector<uint16_t> peds = {1};
    // Determine which zone the viewpoint is in
    auto zone = world->data->findZoneAt(camera.position);
    bool day = (world->state->basic.gameHour >= 8 &&
                world->state->basic.gameHour <= 19);
    int groupid = zone ? (day ? zone->pedGroupDay : zone->pedGroupNight) : 0;
    const auto& group = world->data->pedgroups.at(groupid);
    peds.insert(peds.end(), group.cbegin(), group.cend());

    std::random_device rd;
    std::default_random_engine re(rd());
    std::uniform_int_distribution<> d(0, peds.size() - 1);
    const glm::vec3 kSpawnOffset{0.f, 0.f, 1.f};

    int counter = availablePeds;
    // maxSpawn can be -1 for "as many as possible"
    if (maxSpawn > -1) {
        counter = std::min(availablePeds, maxSpawn);
    }

    for (AIGraphNode* spawn : available) {
        if (spawn->type != AIGraphNode::Pedestrian) {
            continue;
        }
        if (counter > -1) {
            if (counter == 0) {
                break;
            }
            counter--;
        }

        // Spawn a pedestrian from the available pool
        auto ped = world->createPedestrian(peds[d(re)],
                                           spawn->position + kSpawnOffset);
        ped->setLifetime(GameObject::TrafficLifetime);
        ped->controller->setGoal(CharacterController::TrafficWander);
        created.push_back(ped);
    }

    // Find places it's legal to spawn things

    return created;
}

void TrafficDirector::setPopulationLimits(int maxPeds, int maxCars) {
    maximumPedestrians = maxPeds;
    maximumCars = maxCars;
}
