#include <ai/activity/UseItem.hpp>
#include <data/WeaponData.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <items/Weapon.hpp>

bool UseItem::update(CharacterObject *character,
                                 CharacterController *controller) {
    RW_UNUSED(controller);

    if (itemslot >= kMaxInventorySlots) {
        return true;
    }

    // Finds the cycle associated with an anim from the AnimGroup
    /// @todo doesn't need to happen every update..
    auto find_cycle = [&](const std::string &name) {
        if (name == "null") {
            return AnimCycle::Idle;
        }
        for (auto &i : character->animations->animations_) {
            if (i.name == name) return i.id;
        }
        return AnimCycle::Idle;
    };

    auto world = character->engine;
    const auto &weapon = world->data->weaponData.at(itemslot);
    auto &state = character->getCurrentState().weapons[itemslot];
    auto animator = character->animator;
    auto shootcycle = find_cycle(weapon->animation1);
    auto throwcycle = find_cycle(weapon->animation2);

    // Instant hit weapons loop their anim
    // Thrown projectiles have lob / throw.

    // Update player direction
    character->setRotation(
            glm::angleAxis(character->getLook().x, glm::vec3{0.f, 0.f, 1.f}));

    if (state.bulletsClip == 0 && state.bulletsTotal > 0) {
        state.bulletsClip +=
                std::min(int(state.bulletsTotal), weapon->clipSize);
        state.bulletsTotal -= state.bulletsClip;
    }
    bool hasammo = state.bulletsClip > 0;

    if (weapon->fireType == WeaponData::INSTANT_HIT) {
        if (!character->getCurrentState().primaryActive) {
            // Character is no longer firing
            return true;
        }
        if (hasammo) {
            if (character->getCurrentCycle() != shootcycle) {
                character->playCycle(shootcycle);
            }

            auto loopstart = weapon->animLoopStart / 100.f;
            auto loopend = weapon->animLoopEnd / 100.f;
            auto firetime = weapon->animFirePoint / 100.f;

            auto currenttime = animator->getAnimationTime(AnimIndexAction);

            if (currenttime >= firetime && !fired) {
                state.bulletsClip--;
                Weapon::fireHitscan(weapon.get(), character);
                fired = true;
            }
            if (currenttime > loopend) {
                animator->setAnimationTime(AnimIndexAction, loopstart);
                fired = false;
            }
        } else if (animator->isCompleted(AnimIndexAction)) {
            // Should we exit this state when out of ammo?
            return true;
        }
    }
        /// @todo Use Thrown flag instead of project (RPG isn't thrown eg.)
    else if (weapon->fireType == WeaponData::PROJECTILE && hasammo) {
        if (character->getCurrentCycle() == shootcycle) {
            if (character->getCurrentState().primaryActive) {
                power = animator->getAnimationTime(AnimIndexAction) / 0.5f;
            }
            if (animator->isCompleted(AnimIndexAction)) {
                character->playCycle(throwcycle);
            }
        } else if (character->getCurrentCycle() == throwcycle) {
            auto firetime = weapon->animCrouchFirePoint / 100.f;
            auto currID = animator->getAnimationTime(AnimIndexAction);

            if (currID >= firetime && !fired) {
                state.bulletsClip--;
                Weapon::fireProjectile(weapon.get(), character, power);
                fired = true;
            }
            if (animator->isCompleted(AnimIndexAction)) {
                return true;
            }
        } else {
            character->playCycle(shootcycle);
        }
    } else if (weapon->fireType == WeaponData::MELEE) {
        RW_CHECK(weapon->fireType != WeaponData::MELEE,
                 "Melee attacks not implemented");
        return true;
    } else {
        RW_ERROR("Unrecognized fireType: " << weapon->fireType);
        return true;
    }

    return false;
}