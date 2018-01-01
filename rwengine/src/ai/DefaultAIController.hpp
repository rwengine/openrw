#pragma once
#ifndef _DEFAULTAICONTROLLER_HPP_
#define _DEFAULTAICONTROLLER_HPP_
#include <ai/CharacterController.hpp>
#include <random>

struct AIGraphNode;
class DefaultAIController : public CharacterController {
    glm::vec3 gotoPos;

public:
    DefaultAIController()
        : CharacterController() {
    }

    glm::vec3 getTargetPosition() override;

    void update(float dt) override;
};

#endif
