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
	_currentActivity = activity;
	if( _currentActivity == nullptr ) {
		character->enterAction( GTACharacter::Idle );
	}
}

GTAAIController::GTAAIController(GTACharacter* character)
: character(character), _currentActivity(nullptr), _nextActivity(nullptr)
{
	character->controller = this;
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
		std::cout << "Checking" << std::endl;
		if( character->currentActivity == GTACharacter::Idle ) {
			std::cout << "was idle" << std::endl;
			character->enterVehicle(vehicle, seat);
			return true;
		}
	}
	else {
		glm::vec3 target = vehicle->getSeatEntryPosition(seat);
		glm::vec3 targetDirection = target - character->getPosition();

		if( glm::length(targetDirection) <= 0.4f ) {
			std::cout << "enter started" << std::endl;
			entering = true;
			character->enterAction(GTACharacter::VehicleGetIn);
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
