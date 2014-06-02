#include <ai/GTAAIController.hpp>
#include <objects/GTACharacter.hpp>
#include <engine/Animator.hpp>

bool GTAAIController::updateActivity()
{
	if( _currentActivity ) {
		return _currentActivity->update(character, this);
	}

	return false;
}

void GTAAIController::setActivity(GTAAIController::Activity* activity)
{
	if( _currentActivity ) delete _currentActivity;
	_currentActivity = activity;
	if( _currentActivity == nullptr ) {
		// TODO make idle an actual activity or something,
		character->clearTargetPosition();
		character->enterAction( GTACharacter::Idle );
	}
}

GTAAIController::GTAAIController(GTACharacter* character)
: character(character), _currentActivity(nullptr), _nextActivity(nullptr)
{
	character->controller = this;
}

void GTAAIController::skipActivity()
{
	setActivity(nullptr);
}

void GTAAIController::setNextActivity(GTAAIController::Activity* activity)
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

void GTAAIController::update(float dt)
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



bool Activities::GoTo::update(GTACharacter *character, GTAAIController *controller)
{
	/* TODO: Use the ai nodes to navigate to the position */
	glm::vec3 targetDirection = target - character->getPosition();

	if( glm::length(targetDirection) < 0.01f ) {
		character->enterAction(GTACharacter::Idle);
		return true;
	}

	character->setTargetPosition( target );

	glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
	character->rotation = r;
	character->enterAction(GTACharacter::Walk);

	return false;
}

#include <objects/GTAVehicle.hpp>

bool Activities::EnterVehicle::update(GTACharacter *character, GTAAIController *controller)
{
	if( entering ) {
		// TODO: decouple from the character's animator.
		if( character->currentActivity == GTACharacter::VehicleGetIn ) {
			character->enterVehicle(vehicle, seat);
		}
		else if( character->currentActivity == GTACharacter::VehicleOpen ) {
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
			character->enterAction(GTACharacter::VehicleOpen);
		}
		else if( targetDistance > 15.f ) {
			return true; // Give up if the vehicle is too far away.
		}
		else {
			character->setTargetPosition( target );

			glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
			character->rotation = r;
			character->enterAction(GTACharacter::Walk);
		}
	}
	return false;
}


bool Activities::ExitVehicle::update(GTACharacter *character, GTAAIController *controller)
{
	if( character->getCurrentVehicle() == nullptr ) return true;

	if( character->currentActivity == GTACharacter::Idle ) {
		auto vehicle = character->getCurrentVehicle();
		auto exitpos = vehicle->getSeatEntryPosition(character->getCurrentSeat());

		character->enterVehicle(nullptr, 0);
		character->setPosition(exitpos);

		return true;
	}

	character->enterAction(GTACharacter::VehicleGetOut);
	return false;
}
