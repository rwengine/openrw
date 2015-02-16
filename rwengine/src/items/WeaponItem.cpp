#include <items/WeaponItem.hpp>
#include <objects/CharacterObject.hpp>
#include <ai/CharacterController.hpp>
#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <engine/GameWorld.hpp>
#include <objects/ProjectileObject.hpp>
#include <data/Skeleton.hpp>

void WeaponItem::fireHitscan()
{
	auto handFrame = _character->model->model->findFrame("srhand");
	glm::mat4 handMatrix;
	if( handFrame ) {
		while( handFrame->getParent() ) {
			handMatrix = _character->skeleton->getMatrix(handFrame->getIndex()) * handMatrix;
			handFrame = handFrame->getParent();
		}
	}

	auto farTarget = _character->getPosition() +
			_character->getRotation() * glm::vec3(0.f, _wepData->hitRange, 0.f);
	auto handPos = glm::vec3(handMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f));
	auto fireOrigin = _character->getPosition() +
			_character->getRotation() * handPos;
	auto flashDir = _character->getRotation() * glm::vec3{0.f, 0.f, 1.f};
	auto flashUp = _character->getRotation() * glm::vec3{0.f, -1.f, 0.f};

	_character->engine->doWeaponScan(WeaponScan(_wepData->damage, fireOrigin, farTarget, _wepData.get()));

	// Particle FX involved:
	// - smokeII emited around barrel
	// - Some circle particle used for the tracer
	// - smoke emited at hit point
	// - gunflash

	auto tracerTex = _character->engine->gameData.findTexture("shad_exp")->getName();
	auto flashTex = _character->engine->gameData.findTexture("gunflash2")->getName();
	auto flashTex1 = _character->engine->gameData.findTexture("gunflash1")->getName();

	float tracertime = 0.1f;
	auto distance = glm::distance(fireOrigin, farTarget);
	const float tracerspeed = distance / tracertime * 0.5f;
	float tracersize = _wepData->hitRange / 4.f;
	float flashtime = 0.015f;
	auto shotdir = glm::normalize(farTarget - fireOrigin);

	_character->engine->renderer.addParticle({
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
}

void WeaponItem::fireProjectile()
{
	auto handPos = glm::vec3(0.f, 1.5f, 1.f);
	auto fireOrigin = _character->getPosition() +
			_character->getRotation() * handPos;
	auto direction = _character->getRotation() * glm::normalize(glm::vec3{0.f, 1.f, 1.f});

	auto pt = _wepData->name == "grenade" ? ProjectileObject::Grenade : ProjectileObject::Molotov;

	// Work out the velocity multiplier as a function of how long the player
	// Was holding down the fire button. If _fireStop < 0.f then the player
	// is still holding the button down.
	float throwTime = _character->engine->gameTime - _fireStart;
	float forceFactor = throwTime;
	if( _fireStop > 0.f ) {
		forceFactor = _fireStop - _fireStart;
	}
	forceFactor /= throwTime;

	auto projectile = new ProjectileObject(_character->engine, fireOrigin,
	{
											pt,
											direction,
											17.f * forceFactor, /// @todo pull a better velocity from somewhere
											3.5f,
											_wepData
										});

	_character->engine->objects.insert( projectile );
}

void WeaponItem::primary(bool active)
{
	_firing = active;
	if( active ) {
		_fireStart = _character->engine->gameTime;
		_fireStop = -1.f;

		// ShootWeapon will call ::fire() on us at the appropriate time.
		_character->controller->setNextActivity(new Activities::ShootWeapon(this));
	}
	else {
		_fireStop = _character->engine->gameTime;
	}
}

void WeaponItem::secondary(bool active)
{

}

void WeaponItem::fire()
{
	switch( _wepData->fireType ) {
	case WeaponData::INSTANT_HIT:
		fireHitscan();
		break;
	case WeaponData::PROJECTILE:
		fireProjectile();
		break;
	default:
		/// @todo meele
		break;
	}
}
