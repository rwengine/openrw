#include "engine/Payphone.hpp"

#include <rw/debug.hpp>

#include "ai/PlayerController.hpp"

#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"

#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/InstanceObject.hpp"

Payphone::Payphone(GameWorld* engine_, const int id_, const glm::vec2 coord)
    : engine(engine_), id(id_) {
    // Find payphone object, original game does this differently
    for (const auto& p : engine->instancePool.objects) {
        auto o = p.second;
        if (!o->getModel()) {
            continue;
        }
        if (o->getModelInfo<BaseModelInfo>()->name != "phonebooth1") {
            continue;
        }
        if (glm::distance(coord, glm::vec2(o->getPosition())) < 2.f) {
            object = static_cast<InstanceObject*>(o);
            break;
        }
    }

    message.clear();

    if (object) {
        position = object->getPosition();
    }
}

void Payphone::enable() {
    state = State::Ringing;
}

void Payphone::disable() {
    state = State::Idle;
}

bool Payphone::isTalking() const {
    return state == State::Talking;
}

void Payphone::setMessageAndStartRinging(const std::string& m) {
    state = State::Ringing;
    message = m;
}

void Payphone::tick(float dt) {
    RW_UNUSED(dt);

    switch (state) {
        case State::Idle: {
            break;
        }

        case State::Ringing: {
            if (glm::distance(
                    position,
                    engine->getPlayer()->getCharacter()->getPosition()) < 1.f) {
                state = State::PickingUp;

                engine->getPlayer()->pickUpPayphone();

                engine->state->isCinematic = true;
                engine->getPlayer()->prepareForCutscene();

                engine->getPlayer()->getCharacter()->setHeading(glm::degrees(
                    glm::atan(position.x, position.y) + glm::half_pi<float>()));
            }

            // @todo Do wiggle animation

            break;
        }

        case State::PickingUp: {
            if (!engine->getPlayer()->isPickingUpPayphone()) {
                state = State::Talking;

                if (!message.empty()) {
                    const auto& text =
                        ScreenText::format(engine->data->texts.text(message), FONT_ARIAL);

                    engine->state->text.clear<ScreenTextType::HighPriority>();
                    engine->state->text.addText<ScreenTextType::HighPriority>(
                        ScreenTextEntry::makeHighPriority(message, text, 3000));

                    message.clear();
                }

                callTimer = engine->getGameTime() + 3.f;
            }

            break;
        }

        case State::Talking: {
            if (callTimer <= engine->getGameTime()) {
                state = State::HangingUp;

                engine->getPlayer()->hangUpPayphone();
            }

            break;
        }

        case State::HangingUp: {
            if (!engine->getPlayer()->isHangingUpPayphone()) {
                state = State::Idle;

                engine->state->isCinematic = false;
                engine->getPlayer()->freeFromCutscene();
            }

            break;
        }

        default: { break; }
    }
}
