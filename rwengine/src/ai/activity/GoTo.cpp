#include <ai/activity/GoTo.hpp>

bool GoTo::update(CharacterObject *character,
                              CharacterController *controller) {
    /* TODO: Use the ai nodes to navigate to the position */
    auto cpos = character->getPosition();
    glm::vec3 targetDirection = target - cpos;

    // Ignore vertical axis for the sake of simplicity.
    if (glm::length(glm::vec2(targetDirection)) < 0.1f) {
        character->setPosition(glm::vec3(glm::vec2(target), cpos.z));
        controller->setMoveDirection({0.f, 0.f, 0.f});
        character->controller->setRunning(false);
        return true;
    }

    float hdg =
            atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>();
    character->setHeading(glm::degrees(hdg));

    controller->setMoveDirection({1.f, 0.f, 0.f});
    controller->setRunning(sprint);

    return false;
}