#ifndef _RWENGINE_DEFAULTAICONTROLLER_HPP_
#define _RWENGINE_DEFAULTAICONTROLLER_HPP_
#include <glm/glm.hpp>
#include <ai/CharacterController.hpp>

class DefaultAIController : public CharacterController {
    glm::vec3 gotoPos{};

public:
    DefaultAIController()
        : CharacterController() {
    }

    glm::vec3 getTargetPosition() override;

    void update(float dt) override;
};

#endif
