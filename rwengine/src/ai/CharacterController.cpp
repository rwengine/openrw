#include <ai/CharacterController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/Animator.hpp>

bool CharacterController::updateActivity()
{
	if( _currentActivity ) {
		return _currentActivity->update(character, this);
	}

	return false;
}

void CharacterController::setActivity(CharacterController::Activity* activity)
{
	if( _currentActivity ) delete _currentActivity;
	_currentActivity = activity;
	if( _currentActivity == nullptr ) {
		// TODO make idle an actual activity or something,
		character->clearTargetPosition();
		character->enterAction( CharacterObject::Idle );
	}
}

CharacterController::CharacterController(CharacterObject* character)
: character(character), _currentActivity(nullptr), _nextActivity(nullptr)
{
	character->controller = this;
}

void CharacterController::skipActivity()
{
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

void CharacterController::update(float dt)
{
	if( updateActivity() ) {
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

bool Activities::GoTo::update(CharacterObject *character, CharacterController *controller)
{
	/* TODO: Use the ai nodes to navigate to the position */
	glm::vec3 targetDirection = target - character->getPosition();

	if( glm::length(targetDirection) < 0.01f ) {
		character->enterAction(CharacterObject::Idle);
		return true;
	}

	character->setTargetPosition( target );

	glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
	character->rotation = r;
	character->enterAction(CharacterObject::Walk);

	return false;
}


bool Activities::EnterVehicle::update(CharacterObject *character, CharacterController *controller)
{
	// Boats don't have any kind of entry animation unless you're onboard.
	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		character->enterVehicle(vehicle, seat);
		return true;
	}


	if( entering ) {
		// TODO: decouple from the character's animator.
		if( character->currentActivity == CharacterObject::VehicleGetIn ) {
			character->enterVehicle(vehicle, seat);
		}
		else if( character->currentActivity == CharacterObject::VehicleOpen ) {
			// Ensure the player remains aligned with the vehicle
			character->setPosition(vehicle->getSeatEntryPosition(seat));
			character->rotation = vehicle->getRotation();
		}
		else {
			// VehicleGetIn is over, finish activity
			return true;
		}
	}
	else {
		glm::vec3 target = vehicle->getSeatEntryPosition(seat);
		glm::vec3 targetDirection = target - character->getPosition();
		targetDirection.z = 0.f;

		float targetDistance = glm::length(targetDirection);

		if( targetDistance <= 0.4f ) {
			entering = true;
			// Warp character to vehicle orientation
			character->rotation = vehicle->getRotation();
			character->enterAction(CharacterObject::VehicleOpen);
		}
		else if( targetDistance > 15.f ) {
			return true; // Give up if the vehicle is too far away.
		}
		else {
			character->setTargetPosition( target );

			glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
			character->rotation = r;
			character->enterAction(CharacterObject::Walk);
		}
	}
	return false;
}


bool Activities::ExitVehicle::update(CharacterObject *character, CharacterController *controller)
{
	if( character->getCurrentVehicle() == nullptr ) return true;

	auto vehicle = character->getCurrentVehicle();

	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		auto ppos = character->getPosition();
		character->enterVehicle(nullptr, 0);
		character->setPosition(ppos);
		return true;
	}

	if( character->currentActivity == CharacterObject::Idle ) {
		auto exitpos = vehicle->getSeatEntryPosition(character->getCurrentSeat());

		character->enterVehicle(nullptr, 0);
		character->setPosition(exitpos);

		return true;
	}

	character->enterAction(CharacterObject::VehicleGetOut);
	return false;
}
