#include "ai/DefaultAIController.hpp"

#include <limits>
#include <memory>
#include <random>

#include "ai/AIGraph.hpp"
#include "ai/AIGraphNode.hpp"
#include "ai/CharacterController.hpp"

#include "engine/GameWorld.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/VehicleObject.hpp"

glm::vec3 DefaultAIController::getTargetPosition() {
    /*if(targetNode) {
        if(lastNode && character->getCurrentVehicle()) {
            auto nDir = glm::normalize(targetNode->position -
    lastNode->position);
            auto right = glm::cross(nDir, glm::vec3(0.f, 0.f, 1.f));
            return targetNode->position + right * 2.2f;
        }
        return targetNode->position;
    }*/
    return glm::vec3();
}

const float followRadius = 5.f;

void DefaultAIController::update(float dt) {
    switch (currentGoal) {
        case FollowLeader: {
            if (!leader) break;
            if (getCharacter()->getCurrentVehicle()) {
                if (leader->getCurrentVehicle() !=
                    getCharacter()->getCurrentVehicle()) {
                    skipActivity();
                    setNextActivity(std::make_unique<Activities::ExitVehicle>());
                }
                // else we're already in the right spot.
            } else {
                if (leader->getCurrentVehicle()) {
                    setNextActivity(std::make_unique<Activities::EnterVehicle>(
                        leader->getCurrentVehicle(), 1));
                } else {
                    glm::vec3 dir =
                        leader->getPosition() - getCharacter()->getPosition();
                    if (glm::length(dir) > followRadius) {
                        if (glm::distance(gotoPos, leader->getPosition()) >
                            followRadius) {
                            gotoPos =
                                leader->getPosition() +
                                (glm::normalize(-dir) * followRadius * 0.7f);
                            skipActivity();
                            setNextActivity(std::make_unique<Activities::GoTo>(gotoPos));
                        }
                    }
                }
            }
        } break;
        case TrafficWander: {
            if (targetNode) {
                auto targetDistance =
                    glm::vec2(character->getPosition() - targetNode->position);
                if (glm::length(targetDistance) <= 0.1f) {
                    // Assign the next target node
                    auto lastTarget = targetNode;
                    std::random_device rd;
                    std::default_random_engine re(rd());
                    std::uniform_int_distribution<> d(
                        0, lastTarget->connections.size() - 1);
                    targetNode = lastTarget->connections.at(d(re));
                    setNextActivity(std::make_unique<Activities::GoTo>(
                        targetNode->position));
                } else if (getCurrentActivity() == nullptr) {
                    setNextActivity(std::make_unique<Activities::GoTo>(
                        targetNode->position));
                }
            } else {
                // We need to pick an initial node
                auto& graph = getCharacter()->engine->aigraph;
                AIGraphNode* node = nullptr;
                float mindist = std::numeric_limits<float>::max();
                for (const auto& n : graph.nodes) {
                    if (n->type != AIGraphNode::Pedestrian) {
                        continue;
		    }

                    float d = glm::distance(n->position,
                                            getCharacter()->getPosition());
                    if (d < mindist) {
                        node = n;
                        mindist = d;
                    }
                }
                targetNode = node;
            }
        } break;
        case TrafficDriver: {

            // If we don't own a car, become a pedestrian
            if (getCharacter()->getCurrentVehicle() == nullptr)
            {
                targetNode = nullptr;
                nextTargetNode = nullptr;
                lastTargetNode = nullptr;
                currentGoal = TrafficWander;

                // Try to skip the current activity
                if (getCharacter()->controller->getCurrentActivity() != nullptr) {
                    getCharacter()->controller->skipActivity();
                }

                setNextActivity(std::make_unique<Activities::ExitVehicle>());
                break;
            }

            // We have a target
            if (targetNode) {
                // Either we reached the node or we started new, therefore set the next activity 
                if (getCurrentActivity() == nullptr) {
                    // Assign the last target node
                    lastTargetNode = targetNode;

                    // Assign the next target node, either it is already set, 
                    // or we have to find one by ourselves
                    if (nextTargetNode != nullptr) {
                        targetNode = nextTargetNode;
                        nextTargetNode = nullptr;
                    }
                    else {
                        float mindist = std::numeric_limits<float>::max();
                        for (const auto& node : lastTargetNode->connections) {
                            const float distance =
                                getCharacter()->getCurrentVehicle()->isInFront(
                                    node->position);
                            const float lastDistance =
                                getCharacter()->getCurrentVehicle()->isInFront(
                                    lastTargetNode->position);

                            // Make sure, that the next node is in front of us, and farther away then the last node
                            if (distance > 0.f && distance > lastDistance) {
                                const float d = glm::distance(
                                    node->position, getCharacter()
                                                        ->getCurrentVehicle()
                                                        ->getPosition());

                                if (d < mindist) {
                                    targetNode = node;
                                    mindist = d;
                                }
                            }
                        }
                    }

                    // If we haven't found a node, choose one randomly
                    if (!targetNode) {
                        auto& random = getCharacter()->engine->randomEngine;
                        int nodeIndex = std::uniform_int_distribution<>(0, lastTargetNode->connections.size() - 1)(random);
                        targetNode = lastTargetNode->connections.at(nodeIndex);
                    }

                    // Check whether the maximum amount of lanes changed and adjust our lane
                    // @todo we don't know the direction of the street, so for now, choose the bigger value
                    int maxLanes = targetNode->rightLanes > targetNode->leftLanes ? targetNode->rightLanes : targetNode->leftLanes;

                    if (maxLanes < getLane()) {
                        if(maxLanes <= 0) {
                            setLane(1);
                        }
                        else {
                            setLane(maxLanes);
                        }
                    }

                    setNextActivity(std::make_unique<Activities::DriveTo>(
                        targetNode, false));
                }
            }
            else {
                // We need to pick an initial node
                auto& graph = getCharacter()->engine->aigraph;
                AIGraphNode* node = nullptr;
                float mindist = std::numeric_limits<float>::max();

                for (const auto& n : graph.nodes) {
                    // No vehicle node, continue
                    if (n->type != AIGraphNode::Vehicle) {
                        continue;
                    }

                    // The node must be ahead of the vehicle
                    if (getCharacter()->getCurrentVehicle()->isInFront(n->position) < 0.f) {
                        continue;
                    }

                    const float d = glm::distance(
                        n->position,
                        getCharacter()->getCurrentVehicle()->getPosition());

                    if (d < mindist) {
                        node = n;
                        mindist = d;
                    }
                }

                targetNode = node;
		
                // Set the next activity
                if (targetNode) {
                    setNextActivity(std::make_unique<Activities::DriveTo>(
                        targetNode, false));
                }
            }
        } break;
        default:
            break;
    }

    CharacterController::update(dt);
}
