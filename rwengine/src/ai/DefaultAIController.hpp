#ifndef _RWENGINE_DEFAULTAICONTROLLER_HPP_
#define _RWENGINE_DEFAULTAICONTROLLER_HPP_

#include <glm/vec3.hpp>

#include "ai/CharacterController.hpp"

namespace ai {

class DefaultAIController final : public CharacterController {
    glm::vec3 gotoPos{};

public:
    DefaultAIController() : CharacterController() {
    }

    glm::vec3 getTargetPosition() override;

    void update(float dt) override;
};

}  // namespace ai

#endif
