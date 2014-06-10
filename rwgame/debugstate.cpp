#include "debugstate.hpp"
#include "game.hpp"
#include <objects/CharacterObject.hpp>

DebugState::DebugState()
	: _freeLook( false ), _sonicMode( false )
{
	Menu *m = new Menu(getFont());
	m->offset = glm::vec2(50.f, 100.f);
	float entryHeight = 24.f;
	m->addEntry(Menu::lambda("Random Vehicle", [this] {
		auto it = getWorld()->vehicleTypes.begin();
		std::uniform_int_distribution<int> uniform(0, 3);
		for(size_t i = 0, n = uniform(getWorld()->randomEngine); i != n; i++) {
			it++;
		}
		spawnVehicle(it->first);
	}, entryHeight));
	int vehiclesMax = 16, i = 0;
	for( auto& v : getWorld()->vehicleTypes ) {
		if( (i++) > vehiclesMax ) break;
		m->addEntry(Menu::lambda(v.second->handlingID, [=] {
			spawnVehicle(v.first);
		}, entryHeight));
	}
	this->enterMenu(m);
}

void DebugState::enter()
{
	_debugPos = getViewPosition();
	_debugAngles = getViewAngles();
}

void DebugState::exit()
{

}

void DebugState::tick(float dt)
{
	/*if(debugObject) {
		auto p = debugObject->getPosition();
		ss << "Position: " << p.x << " " << p.y << " " << p.z << std::endl;
		ss << "Health: " << debugObject->mHealth << std::endl;
		if(debugObject->model) {
			auto m = debugObject->model;
			ss << "Textures: " << std::endl;
			for(auto it = m->geometries.begin(); it != m->geometries.end();
				++it )
			{
				auto g = *it;
				for(auto itt = g->materials.begin(); itt != g->materials.end();
					++itt)
				{
					for(auto tit = itt->textures.begin(); tit != itt->textures.end();
						++tit)
					{
						ss << " " << tit->name << std::endl;
					}
				}
			}
		}
		if(debugObject->type() == GameObject::Vehicle) {
			GTAVehicle* vehicle = static_cast<GTAVehicle*>(debugObject);
			ss << "ID: " << vehicle->info->handling.ID << std::endl;
		}
	}*/

	if( _freeLook ) {
		float qpi = glm::half_pi<float>();

		sf::Vector2i screenCenter{sf::Vector2i{getWindow().getSize()} / 2};
		sf::Vector2i mousePos = sf::Mouse::getPosition(getWindow());
		sf::Vector2i deltaMouse = mousePos - screenCenter;
		sf::Mouse::setPosition(screenCenter, getWindow());

		_debugAngles.x += deltaMouse.x / 100.0;
		_debugAngles.y += deltaMouse.y / 100.0;

		if (_debugAngles.y > qpi)
			_debugAngles.y = qpi;
		else if (_debugAngles.y < -qpi)
			_debugAngles.y = -qpi;

		glm::quat vR = glm::normalize(glm::angleAxis(_debugAngles.x, glm::vec3{0.f, 0.f, 1.f}));
		vR = vR * glm::angleAxis(_debugAngles.y, glm::vec3(1.f, 0.f, 0.f));

		_debugPos += vR * _movement * dt * (_sonicMode ? 100.f : 10.f);
	}

	setViewParameters( _debugPos, _debugAngles );
}

void DebugState::handleEvent(const sf::Event &e)
{
	switch(e.type) {
	case sf::Event::KeyPressed:
		switch(e.key.code) {
		default: break;
		case sf::Keyboard::Escape:
			StateManager::get().exit();
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
		case sf::Keyboard::F:
			_freeLook = !_freeLook;
			break;
		case sf::Keyboard::LShift:
			_sonicMode = true;
			break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(e.key.code) {
		case sf::Keyboard::W:
		case sf::Keyboard::S:
			_movement.y = 0.f;
			break;
		case sf::Keyboard::A:
		case sf::Keyboard::D:
			_movement.x = 0.f;
			break;
		case sf::Keyboard::LShift:
			_sonicMode = false;
			break;
		default: break;
		}
	default: break;
	}
	State::handleEvent(e);
}

void DebugState::spawnVehicle(unsigned int id)
{
	auto ch = getPlayerCharacter();
	if(! ch) return;

	glm::vec3 fwd = ch->rotation * glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 hit, normal;
	if(hitWorldRay(ch->position + (fwd * 5.f), {0.f, 0.f, -2.f}, hit, normal)) {
		auto spawnpos = hit + normal;
		getWorld()->createVehicle(id, spawnpos, glm::quat());
	}
}
