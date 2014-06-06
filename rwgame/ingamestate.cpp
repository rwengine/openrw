#include "ingamestate.hpp"
#include "game.hpp"
#include "pausestate.hpp"
#include "debugstate.hpp"
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <render/Model.hpp>

IngameState::IngameState()
	: _player(nullptr), _playerCharacter(nullptr)
{
	_playerCharacter = getWorld()->createPedestrian(1, {100.f, 100.f, 25.f});
	_player = new GTAPlayerAIController(_playerCharacter);

	setPlayerCharacter( _playerCharacter );
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

void IngameState::enter()
{

}

void IngameState::exit()
{

}

void IngameState::tick(float dt)
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

	setViewParameters( viewPos + localview, {localX, _lookAngles.y} );
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
		default: break;
		}
		break;
	default: break;
	}
	State::handleEvent(event);
}
