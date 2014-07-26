#include "ingamestate.hpp"
#include "game.hpp"
#include "pausestate.hpp"
#include "debugstate.hpp"
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/ItemPickup.hpp>
#include <render/Model.hpp>
#include <items/WeaponItem.hpp>

IngameState::IngameState(bool test)
	: _player(nullptr), _playerCharacter(nullptr)
{
	if( test ) {
		startTest();
	}
	else {
		getWorld()->runScript("data/main.scm");
	}
}

void IngameState::startTest()
{
	_playerCharacter = getWorld()->createPedestrian(1, {-1000.f, -990.f, 13.f});
	_player = new PlayerController(_playerCharacter);

	setPlayerCharacter( _playerCharacter );

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
	for( auto& vi : getWorld()->vehicleTypes ) {
		switch( vi.first ) {
		case 140: continue;
		case 141: continue;
		}

		auto sp = carPos;
		if( vi.second->type == VehicleData::BOAT ) {
			sp = boatPos;
		}

		auto v = getWorld()->createVehicle(vi.first, sp, glm::quat());

		if( vi.second->type == VehicleData::BOAT ) {
			boatPos -= glm::vec3( 2.f + v->info->handling.dimensions.x, 0.f, 0.f);
		}
		else {
			carPos -= glm::vec3( 2.f + v->info->handling.dimensions.x, 0.f, 0.f);
		}
	}
}

void IngameState::spawnPlayerVehicle()
{
	if(! _player) return;
	glm::vec3 hit, normal;
	if(hitWorldRay(hit, normal)) {

		// Pick random vehicle.
		auto it = getWorld()->vehicleTypes.begin();
		std::uniform_int_distribution<int> uniform(0, 9);
		for(size_t i = 0, n = uniform(getWorld()->randomEngine); i != n; i++) {
			it++;
		}

		auto spawnpos = hit + normal;
		auto vehicle = getWorld()->createVehicle(it->first, spawnpos,
												 glm::quat(glm::vec3(0.f, 0.f, -_lookAngles.x * PiOver180)));
		_playerCharacter->enterVehicle(vehicle, 0);
	}
}

void IngameState::updateView()
{
	float qpi = glm::half_pi<float>();

	sf::Vector2i screenCenter{sf::Vector2i{getWindow().getSize()} / 2};
	sf::Vector2i mousePos = sf::Mouse::getPosition(getWindow());
	sf::Vector2i deltaMouse = mousePos - screenCenter;
	sf::Mouse::setPosition(screenCenter, getWindow());

	_lookAngles.x += deltaMouse.x / 100.0;
	_lookAngles.y += deltaMouse.y / 100.0;

	if (_lookAngles.y > qpi)
		_lookAngles.y = qpi;
	else if (_lookAngles.y < -qpi)
		_lookAngles.y = -qpi;

	float localX = _lookAngles.x;

	float viewDistance = 2.f;
	if( _playerCharacter->getCurrentVehicle() ) {
		auto model = _playerCharacter->getCurrentVehicle()->model;
		for(auto& g : model->model->geometries) {
			viewDistance = std::max(
						(glm::length(g->geometryBounds.center) + g->geometryBounds.radius) * 1.5f,
						viewDistance);
		}

		auto vfwd = _playerCharacter->getCurrentVehicle()->getRotation() * glm::vec3(1.f, 0.f, 0.f);
		localX += atan2( vfwd.y, vfwd.x );
	}

	glm::quat vR = glm::normalize(glm::angleAxis(localX, glm::vec3{0.f, 0.f, 1.f}));
	_player->updateMovementDirection(vR * _movement, _movement);

	glm::vec3 localview;
	float vy = cos(_lookAngles.y);
	localview.x = -sin(-localX) * vy;
	localview.y = -cos(-localX) * vy;
	localview.z = -sin(_lookAngles.y);
	localview *= -viewDistance;

	glm::vec3 viewPos = _playerCharacter->getPosition();
	if(_playerCharacter->getCurrentVehicle()) {
		viewPos = _playerCharacter->getCurrentVehicle()->getPosition();
	}

	btVector3 rayFrom(viewPos.x, viewPos.y, viewPos.z);
	btVector3 rayTo = rayFrom + btVector3(localview.x, localview.y, localview.z);

	btDynamicsWorld::AllHitsRayResultCallback allrr(rayFrom, rayTo);

	getWorld()->dynamicsWorld->rayTest( rayFrom, rayTo, allrr );

	float viewFraction = 1.f;
	btCollisionObject* playobj = _playerCharacter->physObject;
	if( _playerCharacter->getCurrentVehicle() ) {
		playobj = _playerCharacter->getCurrentVehicle()->physBody;
	}
	if(allrr.hasHit()) {
		auto& co = allrr.m_collisionObjects;
		for(int i = 0; i < co.size(); ++i) {
			if( co[i] != playobj && allrr.m_hitFractions[i] <= viewFraction ) {
				viewFraction = allrr.m_hitFractions[i] * 0.9f;
			}
		}
	}

	setViewParameters( viewPos + localview * viewFraction, {localX, _lookAngles.y} );
}

void IngameState::enter()
{

}

void IngameState::exit()
{

}

void IngameState::tick(float dt)
{
	if( getWorld()->state.player ) {
		_player = getWorld()->state.player;
		_playerCharacter = _player->getCharacter();
	}

	if( _player ) {
		updateView();
	}
}

void IngameState::handleEvent(const sf::Event &event)
{
	switch(event.type) {
	case sf::Event::KeyPressed:
		switch(event.key.code) {
		case sf::Keyboard::Escape:
			StateManager::get().enter(new PauseState);
			break;
		case sf::Keyboard::M:
			StateManager::get().enter(new DebugState);
			break;
		case sf::Keyboard::Space:
			if(_playerCharacter) {
				_playerCharacter->jump();
			}
			break;
		case sf::Keyboard::W:
			_movement.y =-1.f;
			break;
		case sf::Keyboard::S:
			_movement.y = 1.f;
			break;
		case sf::Keyboard::A:
			_movement.x = 1.f;
			break;
		case sf::Keyboard::D:
			_movement.x =-1.f;
			break;
		case sf::Keyboard::LShift:
			_player->setRunning(true);
			break;
		case sf::Keyboard::F:
			if(_playerCharacter) {
				if(_playerCharacter->getCurrentVehicle()) {
					_player->exitVehicle();
				}
				else {
					_player->enterNearestVehicle();
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
			_movement.y = 0.f;
			break;
		case sf::Keyboard::A:
		case sf::Keyboard::D:
			_movement.x = 0.f;
			break;
		case sf::Keyboard::LShift:
			_player->setRunning(false);
			break;
		case sf::Keyboard::F12:
			skipTime(120.f);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseButtonPressed:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			_player->useItem(true, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseButtonReleased:
		switch(event.mouseButton.button) {
		case sf::Mouse::Left:
			_player->useItem(false, true);
			break;
		default: break;
		}
		break;
	case sf::Event::MouseWheelMoved:
		_playerCharacter->cycleInventory(event.mouseWheel.delta > 0);
		break;
	default: break;
	}
	State::handleEvent(event);
}
