#include <ai/CharacterController.hpp>
#include <data/Model.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <items/WeaponItem.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/ProjectileObject.hpp>

void WeaponItem::fireHitscan(CharacterObject* owner) {
    auto handFrame = owner->getModel()->findFrame("srhand");
    glm::mat4 handMatrix;
    if (handFrame) {
        while (handFrame->getParent()) {
            handMatrix =
                owner->skeleton->getMatrix(handFrame->getIndex()) * handMatrix;
            handFrame = handFrame->getParent();
        }
    }

    auto farTarget =
        owner->getPosition() +
        owner->getRotation() * glm::vec3(0.f, _wepData->hitRange, 0.f);
    auto handPos = glm::vec3(handMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f));
    auto fireOrigin = owner->getPosition() + owner->getRotation() * handPos;

    owner->engine->doWeaponScan(
        WeaponScan(_wepData->damage, fireOrigin, farTarget, _wepData.get()));

// Particle FX involved:
// - smokeII emited around barrel
// - Some circle particle used for the tracer
// - smoke emited at hit point
// - gunflash
#if 0  // Should be merged into the VisualFX system
	auto flashDir = owner->getRotation() * glm::vec3{0.f, 0.f, 1.f};
	auto flashUp = owner->getRotation() * glm::vec3{0.f, -1.f, 0.f};

	auto tracerTex = owner->engine->data->findTexture("shad_exp")->getName();
	auto flashTex = owner->engine->data->findTexture("gunflash2")->getName();
	auto flashTex1 = owner->engine->data->findTexture("gunflash1")->getName();

	float tracertime = 0.1f;
	auto distance = glm::distance(fireOrigin, farTarget);
	const float tracerspeed = distance / tracertime * 0.5f;
	float tracersize = _wepData->hitRange / 4.f;
	float flashtime = 0.015f;
	auto shotdir = glm::normalize(farTarget - fireOrigin);

	/// @TODO move this into rendering logic.
	/*_character->engine->renderer.addParticle({
												fireOrigin + shotdir * tracersize / 2.f,
												shotdir,
												tracerspeed,
												GameRenderer::FXParticle::UpCamera,
												_character->engine->gameTime, tracertime,
												tracerTex,
												{0.04f, tracersize},
												{0.f, 0.f, 0.f}
											});

	_character->engine->renderer.addParticle({
												fireOrigin,
												flashDir,
												0.f,
												GameRenderer::FXParticle::Free,
												_character->engine->gameTime, flashtime,
												flashTex,
												{0.2f, 0.2f},
												flashUp
											});

	_character->engine->renderer.addParticle({
												fireOrigin + shotdir * 0.1f,
												flashDir,
												0.f,
												GameRenderer::FXParticle::Free,
												_character->engine->gameTime, flashtime,
												flashTex,
												{0.2f, 0.2f},
												flashUp
											});

	_character->engine->renderer.addParticle({
												fireOrigin + shotdir * 0.2f,
												flashDir,
												0.f,
												GameRenderer::FXParticle::Free,
												_character->engine->gameTime, flashtime,
												flashTex1,
												{0.2f, 0.2f},
												flashUp
											});
											*/
#endif
}

void WeaponItem::fireProjectile(CharacterObject* owner) {
    auto handPos = glm::vec3(0.f, 1.5f, 1.f);
    auto fireOrigin = owner->getPosition() + owner->getRotation() * handPos;
    auto direction =
        owner->getRotation() * glm::normalize(glm::vec3{0.f, 1.f, 1.f});

    auto pt = _wepData->name == "grenade" ? ProjectileObject::Grenade
                                          : ProjectileObject::Molotov;

    // Work out the velocity multiplier as a function of how long the player
    // Was holding down the fire button. If _fireStop < 0.f then the player
    // is still holding the button down.
    float throwTime = owner->engine->getGameTime() -
                      owner->getCurrentState().primaryStartTime / 1000.f;
    float forceFactor = throwTime;
    if (owner->getCurrentState().primaryEndTime >=
        owner->getCurrentState().primaryStartTime) {
        uint32_t heldTime = owner->getCurrentState().primaryEndTime -
                            owner->getCurrentState().primaryStartTime;
        forceFactor = (heldTime) / 1000.f;
    }
    forceFactor = std::max(0.1f, forceFactor / throwTime);

    auto projectile = new ProjectileObject(
        owner->engine, fireOrigin,
        {pt, direction,
         17.f * forceFactor,  /// @todo pull a better velocity from somewhere
         3.5f, _wepData});

    auto& pool = owner->engine->getTypeObjectPool(projectile);
    pool.insert(projectile);
    owner->engine->allObjects.push_back(projectile);
}

void WeaponItem::primary(CharacterObject* owner) {
    if (owner->getCurrentState().primaryActive) {
        // ShootWeapon will call ::fire() on us at the appropriate time.
        owner->controller->setNextActivity(new Activities::ShootWeapon(this));
    }
}

void WeaponItem::secondary(CharacterObject* owner) {
    RW_UNUSED(owner);
}

void WeaponItem::fire(CharacterObject* owner) {
    switch (_wepData->fireType) {
        case WeaponData::INSTANT_HIT:
            fireHitscan(owner);
            break;
        case WeaponData::PROJECTILE:
            fireProjectile(owner);
            break;
        default:
            /// @todo meele
            break;
    }
}

bool WeaponItem::isFiring(CharacterObject* owner) {
    return owner->getCurrentState().primaryActive;
}
