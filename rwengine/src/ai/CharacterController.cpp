#include <ai/CharacterController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <items/WeaponItem.hpp>
#include <boost/concept_check.hpp>

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
	}
}

CharacterController::CharacterController(CharacterObject* character)
: character(character), _currentActivity(nullptr), _nextActivity(nullptr), running(false),
currentGoal(None), leader(nullptr)
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
	auto d = rawMovement;
	
	if( character->getCurrentVehicle() ) {
		// Nevermind, the player is in a vehicle.

		if( character->getCurrentSeat() == 0 )
		{
			character->getCurrentVehicle()->setSteeringAngle(d.y);
			
			if( std::abs(d.x) > 0.01f )
			{
				character->getCurrentVehicle()->setHandbraking(false);
			}
			character->getCurrentVehicle()->setThrottle(d.x);
		}

		// if the character isn't doing anything, play sitting anim.
		if( _currentActivity == nullptr ) {
			/// @todo play _low variant if car has low flag.
			character->playAnimation(character->animations.car_sit, true);
			
			// If character is idle in vehicle, close door.
			if( glm::length( d ) <= 0.1f )
			{
				auto entryDoor = character->getCurrentVehicle()->getSeatEntryDoor(character->getCurrentSeat());
				
				if( entryDoor && entryDoor->constraint )
				{
					character->getCurrentVehicle()->setPartTarget(entryDoor, true, entryDoor->closedAngle);
				}
			}
			else
			{
				auto entryDoor = character->getCurrentVehicle()->getSeatEntryDoor(character->getCurrentSeat());
				entryDoor->holdAngle = false;
			}
		}
	}
	else {
		if( d.x > 0.001f ) {
			if( running ) {
				if( character->animator->getAnimation() != character->animations.run ) {
					character->playAnimation(character->animations.run, true);
				}
			}
			else {
				if( character->animator->getAnimation() == character->animations.walk_start ) {
					if( character->animator->isCompleted() ) {
						character->playAnimation(character->animations.walk, true);
					}
				}
				else if( character->animator->getAnimation() != character->animations.walk ) {
					character->playAnimation(character->animations.walk_start, false);
				}
			}
		}
		else if( d.x < -0.001f ) {
			if( character->animator->getAnimation() == character->animations.walk_back_start ) {
				if( character->animator->isCompleted() ) {
					character->playAnimation(character->animations.walk_back, true);
				}
			}
			else if( character->animator->getAnimation() != character->animations.walk_back ) {
				character->playAnimation(character->animations.walk_back_start, false);
			}
		}
		else if( glm::abs(d.y) > 0.001f ) {
			if( d.y < 0.f ) {
				if( character->animator->getAnimation() == character->animations.walk_right_start ) {
					if( character->animator->isCompleted() ) {
						character->playAnimation(character->animations.walk_right, true);
					}
				}
				else if( character->animator->getAnimation() != character->animations.walk_right ) {
					character->playAnimation(character->animations.walk_right_start, false);
				}
			}
			else {
				if( character->animator->getAnimation() == character->animations.walk_left_start ) {
					if( character->animator->isCompleted() ) {
						character->playAnimation(character->animations.walk_left, true);
					}
				}
				else if( character->animator->getAnimation() != character->animations.walk_left ) {
					character->playAnimation(character->animations.walk_left_start, false);
				}
			}
		}

		if( _currentActivity == nullptr ) {
			if( glm::length(d) < 0.001f ) {
				character->playAnimation(character->animations.idle, true);
			}
		}
	}
	
	// Reset raw movement between activity updates.
	setRawMovement(glm::vec3());

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

void CharacterController::useItem(bool active, bool primary)
{
	if( character->getActiveItem() ) {
		if( primary ) {
			character->getActiveItem()->primary(active);
		}
		else {
			character->getActiveItem()->secondary(active);
		}
	}
}

CharacterObject *CharacterController::getCharacter() const
{
	return character;
}

void CharacterController::setRawMovement(const glm::vec3 &movement)
{
	rawMovement = movement;
}

void CharacterController::setRunning(bool run)
{
	running = run;
}


bool Activities::GoTo::update(CharacterObject *character, CharacterController *controller)
{
	/* TODO: Use the ai nodes to navigate to the position */
	auto cpos = character->getPosition();
	glm::vec3 targetDirection = target - cpos;

	// Ignore vertical axis for the sake of simplicity.
	if( glm::length(glm::vec2(targetDirection)) < 0.1f ) {
		character->setPosition(glm::vec3(glm::vec2(target), cpos.z));
		return true;
	}

	glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
	character->rotation = r;

	controller->setRawMovement({1.f, 0.f, 0.f});

	return false;
}

bool Activities::Jump::update(CharacterObject* character, CharacterController* controller)
{
	if( jumped )
	{
		if( character->physCharacter->canJump() )
		{
			character->playAnimation(character->animations.jump_land, false);
			return true;
		}
		if( character->animator->getAnimation() == character->animations.jump_start )
		{
			if( character->animator->isCompleted() )
			{
				character->playAnimation(character->animations.jump_glide, true);
			}
		}
	}
	else
	{
		character->jump();
		
		character->playAnimation( character->animations.jump_start, false );
		
		jumped = true;
	}
	
	return false;
}

bool Activities::EnterVehicle::update(CharacterObject *character, CharacterController *controller)
{
	// Boats don't have any kind of entry animation unless you're onboard.
	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		character->enterVehicle(vehicle, seat);
		return true;
	}

	auto anm_open = character->animations.car_open_lhs;
	auto anm_enter = character->animations.car_getin_lhs;
	
	auto entryDoor = vehicle->getSeatEntryDoor(seat);

	if( entering ) {
		if( character->animator->getAnimation() == anm_open ) {
			if( character->animator->isCompleted() ) {
				character->playAnimation(anm_enter, false);
				character->enterVehicle(vehicle, seat);
			}
			else if( entryDoor && character->animator->getAnimationTime() >= 0.5f )
			{
				vehicle->setPartTarget(entryDoor, true, entryDoor->openAngle);
			}
			else {
				//character->setPosition(vehicle->getSeatEntryPosition(seat));
				character->rotation = vehicle->getRotation();
			}
		}
		else if( character->animator->getAnimation() == anm_enter ) {
			if( character->animator->isCompleted() ) {
				// VehicleGetIn is over, finish activity
				return true;
			}
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
			character->controller->setRawMovement({0.f, 0.f, 0.f});
			character->rotation = vehicle->getRotation();
			
			// Determine if the door open animation should be skipped.
			if( entryDoor == nullptr || (entryDoor->constraint != nullptr && glm::abs(entryDoor->constraint->getHingeAngle()) >= 0.6f ) )
			{
				character->playAnimation(anm_enter, false);
				character->enterVehicle(vehicle, seat);
			}
			else
			{
				character->playAnimation(anm_open, false);
			}
		}
		else if( targetDistance > 15.f ) {
			return true; // Give up if the vehicle is too far away.
		}
		else {
			glm::quat r( glm::vec3{ 0.f, 0.f, atan2(targetDirection.y, targetDirection.x) - glm::half_pi<float>() } );
			character->rotation = r;
			character->controller->setRawMovement({1.f, 0.f, 0.f});
		}
	}
	return false;
}


bool Activities::ExitVehicle::update(CharacterObject *character, CharacterController *controller)
{
	if( character->getCurrentVehicle() == nullptr ) return true;

	auto vehicle = character->getCurrentVehicle();
	auto anm_exit = character->animations.car_getout_lhs;
	auto seat = character->getCurrentSeat();

	if( vehicle->vehicle->type == VehicleData::BOAT ) {
		auto ppos = character->getPosition();
		character->enterVehicle(nullptr, seat);
		character->setPosition(ppos);
		return true;
	}

	if( character->animator->getAnimation() == anm_exit ) {
		if( character->animator->isCompleted() ) {
			auto exitpos = vehicle->getSeatEntryPosition(seat);

			character->enterVehicle(nullptr, seat);
			character->setPosition(exitpos);
			
			return true;
		}
	}
	else {
		character->playAnimation(anm_exit, false);
		
		auto door = vehicle->getSeatEntryDoor(character->getCurrentSeat());
		if( door )
		{
			vehicle->setPartTarget(door, true, door->openAngle);
		}
	}

	return false;
}

#include <engine/GameWorld.hpp>
#include <render/Model.hpp>
bool Activities::ShootWeapon::update(CharacterObject *character, CharacterController *controller)
{
	auto& wepdata = _item->getWeaponData();

	// Instant hit weapons loop their anim
	// Thrown projectiles have lob / throw.

	if( wepdata->fireType == WeaponData::INSTANT_HIT ) {
		if( _item->isFiring() ) {

			auto shootanim = character->engine->gameData.animations[wepdata->animation1];
			if( shootanim ) {
				if( character->animator->getAnimation() != shootanim ) {
					character->playAnimation(shootanim, false);
				}

				auto loopstart = wepdata->animLoopStart / 100.f;
				auto loopend = wepdata->animLoopEnd / 100.f;
				auto firetime = wepdata->animFirePoint / 100.f;

				auto currID = character->animator->getAnimationTime();

				if( currID >= firetime && ! _fired ) {
					_item->fire();
					_fired = true;
				}
				if( currID > loopend ) {
					character->animator->setAnimationTime( loopstart );
					_fired = false;
				}
			}
		}
		else {
			if( character->animator->isCompleted() ) {
				return true;
			}
		}
	}
	/// @todo Use Thrown flag instead of project (RPG isn't thrown eg.)
	else if( wepdata->fireType == WeaponData::PROJECTILE ) {
		auto shootanim = character->engine->gameData.animations[wepdata->animation1];
		auto throwanim = character->engine->gameData.animations[wepdata->animation2];

		if( character->animator->getAnimation() == shootanim ) {
			if( character->animator->isCompleted() ) {
				character->animator->setAnimation(throwanim, false);
			}
		}
		else if( character->animator->getAnimation() == throwanim ) {
			auto firetime = wepdata->animCrouchFirePoint / 100.f;
			auto currID = character->animator->getAnimationTime();

			if( currID >= firetime && !_fired ) {
				_item->fire();
				_fired = true;
			}
			if( character->animator->isCompleted() ) {
				return true;
			}
		}
		else {
			character->animator->setAnimation(throwanim, false);
		}
	}

	return false;
}
