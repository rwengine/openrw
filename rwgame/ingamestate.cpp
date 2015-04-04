#include "ingamestate.hpp"
#include "RWGame.hpp"
#include "pausestate.hpp"
#include "debugstate.hpp"
#include "DrawUI.hpp"

#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/ItemPickup.hpp>
#include <render/Model.hpp>
#include <items/WeaponItem.hpp>
#include <engine/GameWorld.hpp>
#include <script/ScriptMachine.hpp>

#define AUTOLOOK_TIME 2.f

/**
 * This should be kept in rwengine/physics
 */
class ClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
	btCollisionObject* _self;
public:

	ClosestNotMeRayResultCallback( btCollisionObject* self, const btVector3& from, const btVector3& to )
		: ClosestRayResultCallback( from, to ), _self( self ) {}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
	{
		if( rayResult.m_collisionObject == _self ) {
			return 1.0;
		}
		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

IngameState::IngameState(RWGame* game, bool test)
	: State(game), started(false), test(test), autolookTimer(0.f)
{
}

void IngameState::startTest()
{
	auto playerChar = getWorld()->createPedestrian(1, {270.f, -605.f, 40.f});
	auto player = new PlayerController(playerChar);

	getWorld()->state.player = player;

	/*auto bat = new WeaponItem(getWorld()->gameData.weaponData["ak47"]);
	_playerCharacter->addToInventory(bat);
	_playerCharacter->setActiveItem(bat->getInventorySlot());*/

	glm::vec3 itemspawn( 276.5f, -609.f, 36.5f);
	for( auto& w : getWorld()->gameData.weaponData ) {
		if( w.first == "unarmed" ) continue;
		getWorld()->objects.insert(new ItemPickup(getWorld(), itemspawn,
												  w.second));
		itemspawn.x += 2.5f;
	}

	auto carPos = glm::vec3( 286.f, -591.f, 37.f );
	auto carRot = glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
	//auto boatPos = glm::vec3( -1000.f, -1040.f, 5.f );
	int i = 0;
	for( auto& vi : getWorld()->objectTypes ) {
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
	getWorld()->sound.playBackground( getWorld()->gameData.getDataPath() + "/audio/City.wav" );
}

PlayerController *IngameState::getPlayer()
{
	return getWorld()->state.player;
}

void IngameState::enter()
{
	if( ! started )
	{
		if( test ) {
			startTest();
		}
		else {
			startGame();
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
	
	auto player = getPlayer();
	if( player && player->isInputEnabled() )
	{
		float qpi = glm::half_pi<float>();

		sf::Vector2i screenCenter{sf::Vector2i{getWindow().getSize()} / 2};
		sf::Vector2i mousePos = sf::Mouse::getPosition(getWindow());
		sf::Vector2i deltaMouse = mousePos - screenCenter;
		sf::Mouse::setPosition(screenCenter, getWindow());
		
		if(deltaMouse.x != 0 || deltaMouse.y != 0)
		{
			autolookTimer = AUTOLOOK_TIME;
		}

		_lookAngles.x += deltaMouse.x / 100.0;
		_lookAngles.y += deltaMouse.y / 100.0;
		
		while(_lookAngles.x > glm::pi<float>())
		{
			_lookAngles.x -= 2.f * glm::pi<float>();
		}
		while(_lookAngles.x < -glm::pi<float>())
		{
			_lookAngles.x += 2.f * glm::pi<float>();
		}

		if (_lookAngles.y > qpi)
			_lookAngles.y = qpi;
		else if (_lookAngles.y < -qpi)
			_lookAngles.y = -qpi;

		auto angle = glm::angleAxis(-_lookAngles.x, glm::vec3(0.f, 0.f, 1.f));
		angle *= glm::angleAxis(_lookAngles.y, glm::vec3(0.f, 1.f, 0.f));

		player->updateMovementDirection(angle * _movement, _movement);
		
		auto target = getWorld()->state.cameraTarget;
		
		if( target == nullptr )
		{
			target = player->getCharacter();
		}

		auto position = target->getPosition();

		float viewDistance = 4.f;
		btCollisionObject* physTarget = player->getCharacter()->physObject;

		auto vehicle = ( target->type() == GameObject::Character ) ? static_cast<CharacterObject*>(target)->getCurrentVehicle() : nullptr;
		if( vehicle ) {
			auto model = vehicle->model;
			for(auto& g : model->resource->geometries) {
				viewDistance = std::max(
							(glm::length(g->geometryBounds.center) + g->geometryBounds.radius) * 4.0f,
							viewDistance);
			}
			position = vehicle->getPosition();
			position.z += (vehicle->info->handling.dimensions.z / 2.f) * 2.5f;
			physTarget = vehicle->physBody;
			
			float speed = vehicle->physVehicle->getCurrentSpeedKmHour();
			if( autolookTimer <= 0.f && std::abs(speed) > 1.f )
			{
				float b = glm::roll(vehicle->getRotation()) + glm::half_pi<float>();
				while( b > glm::pi<float>() )
				{
					b -= 2.f * glm::pi<float>();
				}
				while( b < -glm::pi<float>() )
				{
					b += 2.f * glm::pi<float>();
				}
				if( speed < 0.f )
				{
					if( _lookAngles.x < 0.f )
					{
						b -= glm::pi<float>();
					}
					else
					{
						b += glm::pi<float>();
					}
				}
				
				float aD = b - _lookAngles.x;
				const float rotateSpeed = 1.f;
				if( std::abs(aD) <= rotateSpeed * dt )
				{
					_lookAngles.x = b;
				}
				else
				{
					_lookAngles.x += glm::sign(aD) * rotateSpeed * dt;
				}
				angle = glm::angleAxis(_lookAngles.x, glm::vec3(0.f, 0.f, 1.f));
			}
		}

		auto rayEnd = position + angle * glm::vec3(-viewDistance, 0.f, 1.f);
		auto rayStart = position + glm::vec3(0.f, 0.f, 1.f);
		auto to = btVector3(rayEnd.x, rayEnd.y, rayEnd.z);
		auto from = btVector3(rayStart.x, rayStart.y, rayStart.z);
		ClosestNotMeRayResultCallback ray(physTarget, from, to);

		getWorld()->dynamicsWorld->rayTest(from, to, ray);
		if( ray.hasHit() && ray.m_closestHitFraction < 1.f )
		{
			position = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
							ray.m_hitPointWorld.z());
			position += glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
							ray.m_hitNormalWorld.z()) * 0.1f;
		}
		else
		{
			position = rayEnd;
		}

		// Move back from the character

		// Tilt the final look angle down a tad.
		angle *= glm::angleAxis(glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));

		_look.position = position;
		_look.rotation = angle;
	}
}

void IngameState::draw(GameRenderer* r)
{
	if( !getWorld()->state.isCinematic && getWorld()->isCutsceneDone() )
	{
		drawHUD(getPlayer(), getWorld(), r);
	}
	
    State::draw(r);
}

void IngameState::handleEvent(const sf::Event &event)
{
	auto player = getPlayer();

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
			if( getWorld()->state.currentCutscene )
			{
				getWorld()->state.skipCutscene = true;
			}
			else if( player && player->isInputEnabled() ) {
				if( player->getCharacter()->getCurrentVehicle() ) {
					player->getCharacter()->getCurrentVehicle()->setHandbraking(true);
				}
				else 
				{
					player->jump();
				}
			}
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
		case sf::Keyboard::LShift:
			player->setRunning(true);
			break;
		case sf::Keyboard::F:
			if( player && player->isInputEnabled() ) {
				if( player->getCharacter()->getCurrentVehicle()) {
					player->exitVehicle();
				}
				else {
					player->enterNearestVehicle();
				}
			}
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
		case sf::Keyboard::LShift:
			player->setRunning(false);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseButtonPressed:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			player->useItem(true, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseButtonReleased:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			player->useItem(false, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseWheelMoved:
		player->getCharacter()->cycleInventory(event.mouseWheel.delta > 0);
		break;
	default: break;
	}
	State::handleEvent(event);
}

bool IngameState::shouldWorldUpdate()
{
    return true;
}

const ViewCamera &IngameState::getCamera()
{
	return _look;
}


