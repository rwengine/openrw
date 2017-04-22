#include <ai/DefaultAIController.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>

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
            if (getCharacter()->isInVehicle()) {
                if (leader->getCurrentVehicle() !=
                    getCharacter()->getCurrentVehicle()) {
                    skipActivity();
                    setNextActivity(new Activities::ExitVehicle);
                }
                // else we're already in the right spot.
            } else {
                if (leader->isInVehicle()) {
                    setNextActivity(new Activities::EnterVehicle(
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
                            setNextActivity(new Activities::GoTo(gotoPos));
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
                    setNextActivity(new Activities::GoTo(targetNode->position));
                } else if (getCurrentActivity() == nullptr) {
                    setNextActivity(new Activities::GoTo(targetNode->position));
                }
            } else {
                // We need to pick an initial node
                auto& graph = getCharacter()->engine->aigraph;
                AIGraphNode* node = nullptr;
                float mindist = std::numeric_limits<float>::max();
                for (auto n : graph.nodes) {
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
        default:
            break;
    }

    CharacterController::update(dt);
}
