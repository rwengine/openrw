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

#define AUTOLOOK_TIME 2.f

IngameState::IngameState(RWGame* game, bool test)
	: State(game), autolookTimer(0.f)
{
	if( test ) {
		startTest();
	}
	else {
		getWorld()->runScript("data/main.scm");
	}
	
	// Start playing city.wav
	
	getWorld()->sound.playBackground( getWorld()->gameData.getDataPath() + "/audio/City.wav" );
}

void IngameState::startTest()
{
	auto playerChar = getWorld()->createPedestrian(1, {-1000.f, -990.f, 13.f});
	auto player = new PlayerController(playerChar);

	getWorld()->state.player = player;

	/*auto bat = new WeaponItem(getWorld()->gameData.weaponData["ak47"]);
	_playerCharacter->addToInventory(bat);
	_playerCharacter->setActiveItem(bat->getInventorySlot());*/

	glm::vec3 itemspawn(-1000.f, -980.f, 11.0f);
	for( auto& w : getWorld()->gameData.weaponData ) {
		if( w.first == "unarmed" ) continue;
		getWorld()->objects.insert(new ItemPickup(getWorld(), itemspawn,
												  w.second));
		itemspawn.x += 2.5f;
	}

	auto carPos = glm::vec3( -1000.f, -1000.f, 12.f );
	auto boatPos = glm::vec3( -1000.f, -1040.f, 5.f );
	for( auto& vi : getWorld()->objectTypes ) {
		switch( vi.first ) {
		case 140: continue;
		case 141: continue;
		}
		if( vi.second->class_type == ObjectInformation::_class("CARS") )
		{
			auto vehicle = std::static_pointer_cast<VehicleData>(vi.second);

			auto sp = carPos;
			if( vehicle->type == VehicleData::BOAT ) {
				sp = boatPos;
			}

			auto v = getWorld()->createVehicle(vi.first, sp, glm::quat());

			if( vehicle->type == VehicleData::BOAT ) {
				boatPos -= glm::vec3( 2.f + v->info->handling.dimensions.x, 0.f, 0.f);
			}
			else {
				carPos -= glm::vec3( 2.f + v->info->handling.dimensions.x, 0.f, 0.f);
			}
		}
	}
}

void IngameState::spawnPlayerVehicle()
{
#if 0
	if(! getWorld()->state.player ) return;
	glm::vec3 hit, normal;
	if(game->hitWorldRay(hit, normal)) {

		// Pick random vehicle.
		auto it = getWorld()->vehicleTypes.begin();
		std::uniform_int_distribution<int> uniform(0, 9);
		for(size_t i = 0, n = uniform(getWorld()->randomEngine); i != n; i++) {
			it++;
		}

		auto spawnpos = hit + normal;
		auto vehicle = getWorld()->createVehicle(it->first, spawnpos,
												 glm::quat(glm::vec3(0.f, 0.f, -_lookAngles.x * PiOver180)));

		getWorld()->state.player->getCharacter()->enterVehicle(vehicle, 0);
	}
#endif
}

PlayerController *IngameState::getPlayer()
{
	return getWorld()->state.player;
}

void IngameState::enter()
{

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

		// Move back from the character
		position += angle * glm::vec3(-viewDistance, 0.f, 1.f);

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

const ViewCamera &IngameState::getCamera()
{
	return _look;
}


