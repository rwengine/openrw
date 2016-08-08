#include <ai/CharacterController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <btBulletDynamicsCommon.h>

#include <data/Model.hpp>
#include <engine/Animator.hpp>
#include <items/WeaponItem.hpp>
#include <rw/defines.hpp>

constexpr float kCloseDoorIdleTime = 2.f;

CharacterController::CharacterController(CharacterObject* character)
	: character(character)
	, _currentActivity(nullptr)
	, _nextActivity(nullptr)
	, m_closeDoorTimer(0.f)
	, currentGoal(None)
	, leader(nullptr)
	, targetNode(nullptr)
{
	character->controller = this;
}

bool CharacterController::updateActivity()
{
	if( _currentActivity && character->isAlive() ) {
		return _currentActivity->update(character, this);
	}

	return false;
}

void CharacterController::setActivity(CharacterController::Activity* activity)
{
	if( _currentActivity ) delete _currentActivity;
	_currentActivity = activity;
}

void CharacterController::skipActivity()
{
	// Some activities can't be cancelled, such as the final phase of entering a vehicle
	// or jumping.
	if (getCurrentActivity() != nullptr &&
			getCurrentActivity()->canSkip(character, this))
		setActivity(nullptr);
}

void CharacterController::setNextActivity(CharacterController::Activity* activity)
{
	if( _currentActivity == nullptr ) {
		setActivity(activity);
		_nextActivity = nullptr;
	}
	else {
		if(_nextActivity) delete _nextActivity;
		_nextActivity = activity;
	}
}

bool CharacterController::isCurrentActivity(const std::string& activity) const
{
	if (getCurrentActivity() == nullptr) return false;
	return getCurrentActivity()->name() == activity;
}

void CharacterController::update(float dt)
{
	if( character->getCurrentVehicle() ) {
		// Nevermind, the player is in a vehicle.

		auto& d = character->getMovement();

		if( character->getCurrentSeat() == 0 )
		{
			character->getCurrentVehicle()->setSteeringAngle(d.y);
			
			if( std::abs(d.x) > 0.01f )
			{
				character->getCurrentVehicle()->setHandbraking(false);
			}
			character->getCurrentVehicle()->setThrottle(d.x);
		}

		if( _currentActivity == nullptr ) {
			// If character is idle in vehicle, try to close the door.
			auto v = character->getCurrentVehicle();
			auto entryDoor = v->getSeatEntryDoor(character->getCurrentSeat());

			if (entryDoor && entryDoor->constraint) {
				if (glm::length( d ) <= 0.1f) {
					if (m_closeDoorTimer >= kCloseDoorIdleTime) {
						character->getCurrentVehicle()->setPartTarget(entryDoor, true, entryDoor->closedAngle);
					}
					m_closeDoorTimer += dt;
				}
				else {
					m_closeDoorTimer = 0.f;
				}
			}
		}
	}
	else
	{
		m_closeDoorTimer = 0.f;
	}

	if( updateActivity() ) {
		character->activityFinished();
		if( _currentActivity ) {
			delete _currentActivity;
			_currentActivity = nullptr;
		}
		if( _nextActivity ) {
			setActivity( _nextActivity );
			_nextActivity = nullptr;
		}
	}
}

CharacterObject *CharacterController::getCharacter() const
{
	return character;
}

void CharacterController::setMoveDirection(const glm::vec3 &movement)
{
	character->setMovement(movement);
}

void CharacterController::setLookDirection(const glm::vec2 &look)
{
	character->setLook(look);
}

void CharacterController::setRunning(bool run)
{
	character->setRunning(run);
}


bool Activities::GoTo::update(CharacterObject *character, CharacterController *controller)
{
	/* TODO: Use the ai nodes to navigate to the position */
	auto cpos = character->getPosition();
	glm::vec3 targetDirection = target - cpos;

	// Ignore vertical axis for the sake of simplicity.
	if( glm::length(glm::vec2(targetDirection)) < 0.1f ) {
		character->setPosition(glm::vec3(glm::vec2(target), cpos.z));
		controller->setMoveDirection({0.f, 0.f, 0.f});
		character->controller->setRunning(false);
		return true;
	}

	float hdg = atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>();
	character->setHeading(glm::degrees(hdg));

	controller->setMoveDirection({1.f, 0.f, 0.f});
	controller->setRunning(sprint);

	return false;
}

bool Activities::Jump::update(CharacterObject* character, CharacterController* controller)
{
	RW_UNUSED(controller);
	if (character->physCharacter == nullptr) return true;

	if( !jumped )
	{
		character->jump();
		jumped = true;
	}
	else
	{
		if (character->physCharacter->canJump()) {
			return true;
		}
	}
	
	return false;
}

bool Activities::EnterVehicle::canSkip(CharacterObject *character, CharacterController *) const
{
	// If we're already inside the vehicle, it can't helped.
	return character->getCurrentVehicle() == nullptr;
}

bool Activities::EnterVehicle::update(CharacterObject *character, CharacterController *controller)
{
	constexpr float kSprintToEnterDistance = 5.f;
	constexpr float kGiveUpDistance = 100.f;

	RW_UNUSED(controller);

	// Boats don't have any kind of entry animation unless you're onboard.
	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		character->enterVehicle(vehicle, seat);
		return true;
	}
	
	if( seat == ANY_SEAT )
	{
		// Determine which seat to take.
		float nearest = std::numeric_limits<float>::max();
		for(unsigned int s = 1; s < vehicle->info->seats.size(); ++s)
		{
			auto entry = vehicle->getSeatEntryPositionWorld(s);
			float dist = glm::distance(entry, character->getPosition());
			if( dist < nearest )
			{
				seat = s;
				nearest = dist;
			}
		}
	}
	
	auto entryDoor = vehicle->getSeatEntryDoor(seat);
	auto entryPos = vehicle->getSeatEntryPositionWorld(seat);

	auto anm_open = character->animations.car_open_lhs;
	auto anm_enter = character->animations.car_getin_lhs;
	auto anm_pullout = character->animations.car_pullout_lhs;

	if( entryDoor->dummy->getDefaultTranslation().x > 0.f )
	{
		anm_open = character->animations.car_open_rhs;
		anm_enter = character->animations.car_getin_rhs;
		anm_pullout = character->animations.car_pullout_rhs;
	}

	// If there's someone in this seat already, we may have to ask them to leave.
	auto currentOccupant= static_cast<CharacterObject*>(vehicle->getOccupant(seat));

	bool tryToEnter = false;
	
	if( entering ) {
		if( character->animator->getAnimation(AnimIndexAction) == anm_open ) {
			if( character->animator->isCompleted(AnimIndexAction) ) {
				tryToEnter = true;
			}
			else if( entryDoor && character->animator->getAnimationTime(AnimIndexAction) >= 0.5f )
			{
				vehicle->setPartTarget(entryDoor, true, entryDoor->openAngle);
			}
			else {
				//character->setPosition(vehicle->getSeatEntryPosition(seat));
				character->rotation = vehicle->getRotation();
			}
		}
		else if (character->animator->getAnimation(AnimIndexAction) == anm_pullout) {
			if (character->animator->isCompleted(AnimIndexAction)) {
				tryToEnter = true;
			}
		}
		else if( character->animator->getAnimation(AnimIndexAction) == anm_enter ) {
			if( character->animator->isCompleted(AnimIndexAction) ) {
				// VehicleGetIn is over, finish activity
				return true;
			}
		}
	}
	else {
		glm::vec3 targetDirection = entryPos - character->getPosition();
		targetDirection.z = 0.f;

		float targetDistance = glm::length(targetDirection);

		if( targetDistance <= 0.4f ) {
			entering = true;
			// Warp character to vehicle orientation
			character->controller->setMoveDirection({0.f, 0.f, 0.f});
			character->controller->setRunning(false);
			character->setHeading(
						glm::degrees(glm::roll(vehicle->getRotation())));
			
			// Determine if the door open animation should be skipped.
			if( entryDoor == nullptr || (entryDoor->constraint != nullptr && glm::abs(entryDoor->constraint->getHingeAngle()) >= 0.6f ) )
			{
				tryToEnter = true;
			}
			else
			{
				character->playActivityAnimation(anm_open, false, true);
			}
		}
		else if (targetDistance > kGiveUpDistance) {
			return true;
		}
		else {
			if( targetDistance > kSprintToEnterDistance ) {
				character->controller->setRunning(true);
			}
			character->setHeading(
						glm::degrees(atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>()));
			character->controller->setMoveDirection({1.f, 0.f, 0.f});
		}
	}

	if (tryToEnter) {
		if (currentOccupant != nullptr && currentOccupant != character) {
			// Play the pullout animation and tell the other character to get out.
			character->playActivityAnimation(anm_pullout, false, true);
			currentOccupant->controller->setNextActivity(new Activities::ExitVehicle(true));
		}
		else {
			character->playActivityAnimation(anm_enter, false, true);
			character->enterVehicle(vehicle, seat);
		}
	}
	return false;
}


bool Activities::ExitVehicle::update(CharacterObject *character, CharacterController *controller)
{
	RW_UNUSED(controller);

	if (jacked) {
		auto anm_jacked_lhs = character->animations.car_jacked_lhs;
		auto anm_jacked_rhs = character->animations.car_jacked_lhs;
		auto anm_current = character->animator->getAnimation(AnimIndexAction);

		if (anm_current == anm_jacked_lhs || anm_current == anm_jacked_rhs) {
			if (character->animator->isCompleted(AnimIndexAction)) {
				return true;
			}
		}
		else {
			if (character->getCurrentVehicle() == nullptr) return true;

			auto vehicle = character->getCurrentVehicle();
			auto seat = character->getCurrentSeat();
			auto door = vehicle->getSeatEntryDoor(seat);

			character->rotation = vehicle->getRotation();

			// Exit the vehicle immediatley
			auto exitpos = vehicle->getSeatEntryPosition(seat);
			character->enterVehicle(nullptr, seat);
			character->setPosition(exitpos);

			if (door->dummy->getDefaultTranslation().x > 0.f) {
				character->playActivityAnimation(anm_jacked_rhs, false, true);
			}
			else {
				character->playActivityAnimation(anm_jacked_lhs, false, true);
			}
			// No need to open the door, it should already be open.
		}
		return false;
	}

	if( character->getCurrentVehicle() == nullptr ) return true;

	auto vehicle = character->getCurrentVehicle();
	auto seat = character->getCurrentSeat();
	auto door = vehicle->getSeatEntryDoor(seat);

	auto anm_exit = character->animations.car_getout_lhs;

	if( door->dummy->getDefaultTranslation().x > 0.f )
	{
		anm_exit = character->animations.car_getout_rhs;
	}

	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		auto ppos = character->getPosition();
		character->enterVehicle(nullptr, seat);
		character->setPosition(ppos);
		return true;
	}

	bool isDriver = vehicle->isOccupantDriver(character->getCurrentSeat());

	// If the vehicle is going too fast, slow down
	if (isDriver)
	{
		if (!vehicle->canOccupantExit())
		{
			vehicle->setBraking(1.f);
			return false;
		}
	}

	if( character->animator->getAnimation(AnimIndexAction) == anm_exit ) {
		if( character->animator->isCompleted(AnimIndexAction) ) {
			auto exitpos = vehicle->getSeatEntryPositionWorld(seat);

			character->enterVehicle(nullptr, seat);
			character->setPosition(exitpos);

			if (isDriver)
			{
				// Apply the handbrake
				vehicle->setHandbraking(true);
				vehicle->setThrottle(0.f);
			}

			return true;
		}
	}
	else {
		character->playActivityAnimation(anm_exit, false, true);
		if( door )
		{
			vehicle->setPartTarget(door, true, door->openAngle);
		}
	}

	return false;
}

#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <data/Model.hpp>
bool Activities::ShootWeapon::update(CharacterObject *character, CharacterController *controller)
{
	RW_UNUSED(controller);

	auto& wepdata = _item->getWeaponData();

	// Instant hit weapons loop their anim
	// Thrown projectiles have lob / throw.

	// Update player direction
	character->setRotation(glm::angleAxis(character->getLook().x, glm::vec3{0.f, 0.f, 1.f}));

	RW_CHECK(wepdata->inventorySlot < maxInventorySlots, "Inventory slot out of bounds");
	auto& itemState = character->getCurrentState().weapons[wepdata->inventorySlot];
	if (itemState.bulletsClip == 0 && itemState.bulletsTotal > 0) {
		itemState.bulletsClip += std::min(int(itemState.bulletsTotal), wepdata->clipSize);
		itemState.bulletsTotal -= itemState.bulletsClip;
	}
	bool hasammo = itemState.bulletsClip > 0;

	if( wepdata->fireType == WeaponData::INSTANT_HIT ) {
		if( _item->isFiring(character) && hasammo ) {

			auto shootanim = character->engine->data->animations[wepdata->animation1];
			if( shootanim ) {
				if( character->animator->getAnimation(AnimIndexAction) != shootanim ) {
					character->playActivityAnimation(shootanim, false, false);
				}

				auto loopstart = wepdata->animLoopStart / 100.f;
				auto loopend = wepdata->animLoopEnd / 100.f;
				auto firetime = wepdata->animFirePoint / 100.f;

				auto currID = character->animator->getAnimationTime(AnimIndexAction);

				if( currID >= firetime && ! _fired ) {
					itemState.bulletsClip --;
					_item->fire(character);
					_fired = true;
				}
				if( currID > loopend ) {
					character->animator->setAnimationTime( AnimIndexAction, loopstart );
					_fired = false;
				}
			}
		}
		else {
			if( character->animator->isCompleted(AnimIndexAction) ) {
				return true;
			}
		}
	}
	/// @todo Use Thrown flag instead of project (RPG isn't thrown eg.)
	else if( wepdata->fireType == WeaponData::PROJECTILE && hasammo ) {
		auto shootanim = character->engine->data->animations[wepdata->animation1];
		auto throwanim = character->engine->data->animations[wepdata->animation2];

		if( character->animator->getAnimation(AnimIndexAction) == shootanim ) {
			if( character->animator->isCompleted(AnimIndexAction) ) {
				character->playActivityAnimation(throwanim, false, false);
			}
		}
		else if( character->animator->getAnimation(AnimIndexAction) == throwanim ) {
			auto firetime = wepdata->animCrouchFirePoint / 100.f;
			auto currID = character->animator->getAnimationTime(AnimIndexAction);

			if( currID >= firetime && !_fired ) {
				itemState.bulletsClip --;
				_item->fire(character);
				_fired = true;
			}
			if( character->animator->isCompleted(AnimIndexAction) ) {
				return true;
			}
		}
		else {
			character->playActivityAnimation(throwanim, false, true);
		}
	}
	else if( wepdata->fireType == WeaponData::MELEE ) {
		RW_CHECK(wepdata->fireType != WeaponData::MELEE, "Melee attacks not implemented");
		return true;
	}
	else
	{
		RW_ERROR("Unrecognized fireType: " << wepdata->fireType);
		return true;
	}


	return false;
}
