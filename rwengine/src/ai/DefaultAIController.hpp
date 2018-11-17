#ifndef _RWENGINE_DEFAULTAICONTROLLER_HPP_
#define _RWENGINE_DEFAULTAICONTROLLER_HPP_
#include <ai/CharacterController.hpp>
#include <glm/glm.hpp>

class DefaultAIController final : public CharacterController {
    glm::vec3 gotoPos{};

public:
    DefaultAIController() = default;

    glm::vec3 getTargetPosition() override;

    void update(float dt) override;
};

#endif
