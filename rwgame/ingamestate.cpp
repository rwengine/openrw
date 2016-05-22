#include "ingamestate.hpp"
#include "RWGame.hpp"
#include "pausestate.hpp"
#include "debugstate.hpp"
#include "DrawUI.hpp"

#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/ItemPickup.hpp>
#include <data/Model.hpp>
#include <items/WeaponItem.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameState.hpp>
#include <script/ScriptMachine.hpp>
#include <dynamics/RaycastCallbacks.hpp>

constexpr float kAutoLookTime = 2.f;
constexpr float kAutolookMinVelocity = 0.2f;
const float kMaxRotationRate = glm::half_pi<float>();
const float kCameraPitchLimit = glm::quarter_pi<float>() * 0.5f;

IngameState::IngameState(RWGame* game, bool newgame, const std::string& save)
	: State(game)
	, started(false)
	, save(save)
	, newgame(newgame)
	, autolookTimer(0.f)
	, camMode(IngameState::CAMERA_NORMAL)
	, m_cameraAngles { 0.f, glm::half_pi<float>() }
{
}

void IngameState::startTest()
{
	auto playerChar = getWorld()->createPlayer({270.f, -605.f, 40.f});
	auto player = new PlayerController(playerChar);

	getWorld()->state->playerObject = playerChar->getGameObjectID();

	/*auto bat = new WeaponItem(getWorld()->data.weaponData["ak47"]);
	_playerCharacter->addToInventory(bat);
	_playerCharacter->setActiveItem(bat->getInventorySlot());*/

	glm::vec3 itemspawn( 276.5f, -609.f, 36.5f);
	for(int i = 1; i < maxInventorySlots; ++i) {
		ItemPickup* pickup =
					new ItemPickup(
						getWorld(),
						itemspawn,
						getWorld()->getInventoryItem(i));
		getWorld()->pickupPool.insert(pickup);
		getWorld()->allObjects.push_back(pickup);
		itemspawn.x += 2.5f;
	}

	auto carPos = glm::vec3( 286.f, -591.f, 37.f );
	auto carRot = glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
	//auto boatPos = glm::vec3( -1000.f, -1040.f, 5.f );
	int i = 0;
	for( auto& vi : getWorld()->data->objectTypes ) {
		switch( vi.first ) {
		case 140: continue;
		case 141: continue;
		}
		if( vi.second->class_type == ObjectInformation::_class("CARS") )
		{
			if ( i++ > 20 ) break;
			auto vehicle = std::static_pointer_cast<VehicleData>(vi.second);

			auto& sp = carPos;
			auto& sr = carRot;
			auto v = getWorld()->createVehicle(vi.first, sp, sr);

			sp += sr * glm::vec3( 2.f + v->info->handling.dimensions.x, 0.f, 0.f);
		}
	}
}

void IngameState::startGame()
{
	game->startScript("data/main.scm");
	game->getScript()->startThread(0);
	getWorld()->sound.playBackground( getWorld()->data->getDataPath() + "/audio/City.wav" );
}

void IngameState::enter()
{
	if( ! started )
	{
		if( newgame )
		{
            if( save.empty() )
            {
                startGame();
            }
            else if( save == "test" )
            {
				startTest();
			}
			else {
                game->loadGame( save );
			}
		}
		started = true;
	}
}

void IngameState::exit()
{

}

void IngameState::tick(float dt)
{
	autolookTimer = std::max(autolookTimer - dt, 0.f);

	auto player = game->getPlayer();
	if( player && player->isInputEnabled() )
	{
		sf::Vector2f screenSize(getWindow().getSize());
		sf::Vector2f screenCenter(screenSize / 2.f);
		sf::Vector2f mouseMove;
		if (game->hasFocus())
		{
			sf::Vector2f mousePos(sf::Mouse::getPosition(getWindow()));
			sf::Vector2f deltaMouse = (mousePos - screenCenter);
			mouseMove = sf::Vector2f(deltaMouse.x / screenSize.x, deltaMouse.y / screenSize.y);
			sf::Mouse::setPosition(sf::Vector2i(screenCenter), getWindow());

			if(deltaMouse.x != 0 || deltaMouse.y != 0)
			{
				autolookTimer = kAutoLookTime;
				m_cameraAngles += glm::vec2(mouseMove.x, mouseMove.y);
				m_cameraAngles.y = glm::clamp(m_cameraAngles.y, kCameraPitchLimit, glm::pi<float>() - kCameraPitchLimit);
			}
		}

		float viewDistance = 4.f;
		switch( camMode )
		{
		case IngameState::CAMERA_CLOSE:
			viewDistance = 2.f;
			break;
		case IngameState::CAMERA_NORMAL:
			viewDistance = 4.0f;
			break;
		case IngameState::CAMERA_FAR:
			viewDistance = 6.f;
			break;
		case IngameState::CAMERA_TOPDOWN:
			viewDistance = 15.f;
			break;
		default:
			viewDistance = 4.f;
		}
		
		auto target = getWorld()->pedestrianPool.find(getWorld()->state->cameraTarget);

		if( target == nullptr )
		{
			target = player->getCharacter();
		}

		glm::vec3 targetPosition = target->getPosition();
		glm::vec3 lookTargetPosition = targetPosition;
		targetPosition += glm::vec3(0.f, 0.f, 1.f);
		lookTargetPosition += glm::vec3(0.f, 0.f, 0.5f);

		btCollisionObject* physTarget = player->getCharacter()->physObject;

		auto vehicle = ( target->type() == GameObject::Character ) ? static_cast<CharacterObject*>(target)->getCurrentVehicle() : nullptr;
		if( vehicle ) {
			auto model = vehicle->model;
			float maxDist = 0.f;
			for(auto& g : model->resource->geometries) {
				float partSize = glm::length(g->geometryBounds.center) + g->geometryBounds.radius;
				maxDist = std::max(partSize, maxDist);
			}
			viewDistance = viewDistance + maxDist;
			targetPosition = vehicle->getPosition();
			lookTargetPosition = targetPosition;
			lookTargetPosition.z += (vehicle->info->handling.dimensions.z);
			targetPosition.z += (vehicle->info->handling.dimensions.z * 2.f);
			physTarget = vehicle->physBody;
			m_cameraAngles.y = glm::half_pi<float>();

			// Rotate the camera to the ideal angle if the player isn't moving it
			float velocity = vehicle->getVelocity();
			if (autolookTimer <= 0.f && glm::abs(velocity) > kAutolookMinVelocity)
			{
				auto idealAngle = -glm::roll(vehicle->getRotation()) - glm::half_pi<float>();
				if (velocity < 0.f) {
					idealAngle = glm::mod(idealAngle - glm::pi<float>(), glm::pi<float>() * 2.f);
				}
				float currentAngle = glm::mod(m_cameraAngles.x, glm::pi<float>()*2);
				float rotation = idealAngle - currentAngle;
				if (glm::abs(rotation) > glm::pi<float>()) {
					rotation -= glm::sign(rotation) * glm::pi<float>()*2.f;
				}
				m_cameraAngles.x += glm::sign(rotation) * std::min(kMaxRotationRate * dt, glm::abs(rotation));
			}
		}

		// Non-topdown camera can orbit
		if( camMode != IngameState::CAMERA_TOPDOWN )
		{
			// Determine the "ideal" camera position for the current view angles
			auto yaw = glm::angleAxis(m_cameraAngles.x, glm::vec3(0.f, 0.f,-1.f));
			auto pitch = glm::angleAxis(m_cameraAngles.y, glm::vec3(0.f,-1.f, 0.f));
			auto cameraOffset =
					yaw * pitch * glm::vec3(0.f, 0.f, viewDistance);
			cameraPosition = targetPosition + cameraOffset;
		}
		else
		{
			cameraPosition = targetPosition + glm::vec3(0.f, 0.f, viewDistance);
		}

		glm::quat angle;

		auto camtotarget = targetPosition - cameraPosition;
		auto dir = glm::normalize(camtotarget);
		float correction = glm::length(camtotarget) - viewDistance;
		if( correction < 0.f )
		{
			float innerDist = viewDistance * 0.1f;
			correction = glm::min(0.f, correction + innerDist);
		}
		cameraPosition += dir * 10.f * correction * dt;

		auto lookdir = glm::normalize(lookTargetPosition - cameraPosition);
		// Calculate the yaw to look at the target.
		float angleYaw = glm::atan(lookdir.y, lookdir.x);
		angle = glm::quat( glm::vec3(0.f, 0.f, angleYaw) );

		// Update player with camera yaw
		if( player->isInputEnabled() )
		{
			if (player->getCharacter()->getCurrentVehicle())
			{
				player->setMoveDirection(_movement);
			}
			else
			{
				float length = glm::length(_movement);
				float movementAngle = angleYaw - M_PI/2.f;
				if (length > 0.1f)
				{
					glm::vec3 direction = glm::normalize(_movement);
					movementAngle += atan2(direction.y, direction.x);
					player->setMoveDirection(glm::vec3(1.f, 0.f, 0.f));
				}
				else
				{
					player->setMoveDirection(glm::vec3(0.f));
				}
				if (player->getCharacter()->canTurn())
				{
					player->getCharacter()->rotation =
							glm::angleAxis(movementAngle, glm::vec3(0.f, 0.f, 1.f));
				}
			}
		}
		else
		{
			player->setMoveDirection(glm::vec3(0.f));
		}

		float len2d = glm::length(glm::vec2(lookdir));
		float anglePitch = glm::atan(lookdir.z, len2d);
		angle *= glm::quat( glm::vec3(0.f, -anglePitch, 0.f) );

		// Use rays to ensure target is visible from cameraPosition
		auto rayEnd = cameraPosition;
		auto rayStart = targetPosition;
		auto to = btVector3(rayEnd.x, rayEnd.y, rayEnd.z);
		auto from = btVector3(rayStart.x, rayStart.y, rayStart.z);
		ClosestNotMeRayResultCallback ray(physTarget, from, to);

		getWorld()->dynamicsWorld->rayTest(from, to, ray);
		if( ray.hasHit() && ray.m_closestHitFraction < 1.f )
		{
			cameraPosition = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
							ray.m_hitPointWorld.z());
			cameraPosition += glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
							ray.m_hitNormalWorld.z()) * 0.1f;
		}

		_look.position = cameraPosition;
		_look.rotation = angle;
	}
}

void IngameState::draw(GameRenderer* r)
{
	if( !getWorld()->state->isCinematic && getWorld()->isCutsceneDone() )
	{
		drawHUD(_look, game->getPlayer(), getWorld(), r);
	}
	
    State::draw(r);
}

void IngameState::handleEvent(const sf::Event &event)
{
	auto player = game->getPlayer();

	switch(event.type) {
	case sf::Event::KeyPressed:
		switch(event.key.code) {
		case sf::Keyboard::Escape:
			StateManager::get().enter(new PauseState(game));
			break;
		case sf::Keyboard::M:
			StateManager::get().enter(new DebugState(game, _look.position, _look.rotation));
			break;
		case sf::Keyboard::Space:
			if( getWorld()->state->currentCutscene )
			{
				getWorld()->state->skipCutscene = true;
			}
			break;
		case sf::Keyboard::C:
			camMode = CameraMode((camMode+(CameraMode)1)%CAMERA_MAX);
			break;
		case sf::Keyboard::W:
			_movement.x = 1.f;
			break;
		case sf::Keyboard::S:
			_movement.x =-1.f;
			break;
		case sf::Keyboard::A:
			_movement.y = 1.f;
			break;
		case sf::Keyboard::D:
			_movement.y =-1.f;
			break;
		default: break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
		case sf::Keyboard::W:
		case sf::Keyboard::S:
			_movement.x = 0.f;
			break;
		case sf::Keyboard::A:
		case sf::Keyboard::D:
			_movement.y = 0.f;
			break;
		default: break;
		}
		break;
	default: break;
	}
	
	if( player && player->isInputEnabled() )
	{
		handlePlayerInput(event);
	}
	State::handleEvent(event);
}

void IngameState::handlePlayerInput(const sf::Event& event)
{
	auto player = game->getPlayer();
	switch(event.type) {
	case sf::Event::KeyPressed:
		switch(event.key.code) {
		case sf::Keyboard::Space:
			if( player->getCharacter()->getCurrentVehicle() ) {
				player->getCharacter()->getCurrentVehicle()->setHandbraking(true);
			}
			else
			{
				player->jump();
			}
			break;
		case sf::Keyboard::LShift:
			player->setRunning(true);
			break;
		case sf::Keyboard::F:
			if( player->getCharacter()->getCurrentVehicle()) {
				player->exitVehicle();
			}
			else {
				player->enterNearestVehicle();
			}
			break;
		default:
			break;
		}
	break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
		case sf::Keyboard::LShift:
			player->setRunning(false);
			break;
		default: break;
		}
	break;
	case sf::Event::MouseButtonPressed:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			player->getCharacter()->useItem(true, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseButtonReleased:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			player->getCharacter()->useItem(false, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseWheelMoved:
		player->getCharacter()->cycleInventory(event.mouseWheel.delta > 0);
		break;
	default:
		break;
	}
}


bool IngameState::shouldWorldUpdate()
{
    return true;
}

const ViewCamera &IngameState::getCamera()
{
	return _look;
}

