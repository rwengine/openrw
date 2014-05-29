#include <ai/GTAAIController.hpp>
#include <objects/GTACharacter.hpp>

bool GTAAIController::updateActivity()
{
	switch( _currentActivity ) {
	default: break;

	case GoTo: {
		/* TODO: Use the ai nodes to navigate to the position */
		glm::vec3 targetDirection = _currentParameter.position - character->getPosition();

		if( glm::length(targetDirection) < 0.01f ) {
			character->enterAction(GTACharacter::Idle);
			return true;
		}

		character->setTargetPosition( _currentParameter.position );

		glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
		character->rotation = r;
		character->enterAction(GTACharacter::Walk);
	} break;

	}
	return false;
}

void GTAAIController::setActivity(GTAAIController::Activity activity, const ActivityParameter &parameter)
{
	_currentActivity = activity;
	_currentParameter = parameter;
}

GTAAIController::GTAAIController(GTACharacter* character)
: character(character), _currentActivity(Idle), _nextActivity(Idle)
{
	character->controller = this;
}

void GTAAIController::setNextActivity(GTAAIController::Activity activity, const ActivityParameter &parameter)
{
	if( _currentActivity == Idle ) {
		setActivity(activity, parameter);
		_nextActivity = Idle;
	}
	else {
		_nextParameter = parameter;
		_nextActivity = activity;
	}
}

void GTAAIController::update(float dt)
{
	if(updateActivity()) {
		setActivity(Idle, {});
	}
}

