#include "debugstate.hpp"
#include "RWGame.hpp"
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameState.hpp>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

void jumpCharacter(RWGame* game, CharacterObject* player, const glm::vec3& target)
{
	glm::vec3 ground = game->getWorld()->getGroundAtPosition(target);
	if( player )
	{
		if( player->getCurrentVehicle() )
		{
			player->getCurrentVehicle()->setPosition(ground + glm::vec3(0.f, 0.f, 1.f));
		}
		else
		{
			player->setPosition(ground + glm::vec3(0.f, 0.f, 1.f));
		}
	}
}

DebugState::DebugState(RWGame* game, const glm::vec3& vp, const glm::quat& vd)
	: State(game), _freeLook( false ), _sonicMode( false )
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2(10.f, 50.f);
	float entryHeight = 14.f;
#if 0
	m->addEntry(Menu::lambda("Random Vehicle", [this] {
		auto it = getWorld()->vehicleTypes.begin();
		std::uniform_int_distribution<int> uniform(0, 3);
		for(size_t i = 0, n = uniform(getWorld()->randomEngine); i != n; i++) {
			it++;
		}
		spawnVehicle(it->first);
	}, entryHeight));
	m->addEntry(Menu::lambda("Open All Doors/Flaps", [=] {
		auto pc = getWorld()->state->player->getCharacter();
		auto pv = pc->getCurrentVehicle();
		if( pv ) {
			for(auto& it : pv->_hingedObjects) {
				pv->setHingeLocked(it.first, false);
			}
		}
	}, entryHeight));

	m->addEntry(Menu::lambda("Spawn Pedestrians", [=] {
		glm::vec3 hit, normal;
		if(game->hitWorldRay(hit, normal)) {
			glm::vec3 spawnPos = hit + glm::vec3(-5, 0.f, 0.0) + normal;
			size_t k = 1;
			// Spawn every pedestrian.
			for(auto it = getWorld()->pedestrianTypes.begin();
				it != getWorld()->pedestrianTypes.end(); ++it) {
				getWorld()->createPedestrian(it->first, spawnPos);
				spawnPos += glm::vec3(2.5, 0, 0);
				if((k++ % 6) == 0) { spawnPos += glm::vec3(-15, -2.5, 0); }
			}
		}
	}, entryHeight));

	int vehiclesMax = 16, i = 0;
	for( auto& v : getWorld()->vehicleTypes ) {
		if( (i++) > vehiclesMax ) break;
		m->addEntry(Menu::lambda(v.second->handlingID, [=] {
			spawnVehicle(v.first);
		}, entryHeight));
	}
#endif
	m->addEntry(Menu::lambda("Add car", [=] {
		auto playerRot = game->getPlayer()->getCharacter()->getRotation();
		auto spawnPos = game->getPlayer()->getCharacter()->getPosition();
		spawnPos += playerRot * glm::vec3(0.f, 3.f, 0.f);
		auto spawnRot = glm::quat(glm::vec3(0.f, 0.f, glm::roll(playerRot) + glm::half_pi<float>()));
		auto car = game->getWorld()->createVehicle(136, spawnPos, spawnRot);
	}, entryHeight));
	m->addEntry(Menu::lambda("Quicksave", [=] {
		game->saveGame("quicksave");
	}, entryHeight));
	m->addEntry(Menu::lambda("Quickload", [=] {
		game->loadGame("quicksave");
	}, entryHeight));
	m->addEntry(Menu::lambda("Jump to Garage", [=] {
		jumpCharacter(game, game->getPlayer()->getCharacter(), glm::vec3(270.f, -605.f, 40.f));
	}, entryHeight));
	m->addEntry(Menu::lambda("Jump to Airport", [=] {
		jumpCharacter(game, game->getPlayer()->getCharacter(), glm::vec3(-950.f, -980.f, 12.f));
	}, entryHeight));
	m->addEntry(Menu::lambda("Jump to Hideout", [=] {
		jumpCharacter(game, game->getPlayer()->getCharacter(), glm::vec3(875.0, -309.0, 100.0));
	}, entryHeight));
	m->addEntry(Menu::lambda("Jump to Luigi's", [=] {
		jumpCharacter(game, game->getPlayer()->getCharacter(), glm::vec3(902.75, -425.56, 100.0));
	}, entryHeight));
	m->addEntry(Menu::lambda("Jump to Hospital", [=] {
		jumpCharacter(game, game->getPlayer()->getCharacter(), glm::vec3(1123.77, -569.15, 100.0));
	}, entryHeight));
	m->addEntry(Menu::lambda("Add Follower", [=] {
		auto spawnPos = game->getPlayer()->getCharacter()->getPosition();
		spawnPos += game->getPlayer()->getCharacter()->getRotation() * glm::vec3(-1.f, 0.f, 0.f);
		auto follower = game->getWorld()->createPedestrian(12, spawnPos);
		jumpCharacter(game, follower, spawnPos);
		follower->controller->setGoal(CharacterController::FollowLeader);
		follower->controller->setTargetCharacter(game->getPlayer()->getCharacter());
	}, entryHeight));
	m->addEntry(Menu::lambda("Set Super Jump", [=] {
		game->getPlayer()->getCharacter()->setJumpSpeed(20.f);
	}, entryHeight));
	m->addEntry(Menu::lambda("Set Normal Jump", [=] {
		game->getPlayer()->getCharacter()->setJumpSpeed(CharacterObject::DefaultJumpSpeed);
	}, entryHeight));
	m->addEntry(Menu::lambda("Full Health", [=] {
		game->getPlayer()->getCharacter()->getCurrentState().health = 100.f;
	}, entryHeight));
	m->addEntry(Menu::lambda("Full Armour", [=] {
		game->getPlayer()->getCharacter()->getCurrentState().armour = 100.f;
	}, entryHeight));
	m->addEntry(Menu::lambda("Cull Here", [=] {
		game->getRenderer()->setCullOverride(true, _debugCam);
	}, entryHeight));

	this->enterMenu(m);

	_debugCam.position = vp;
	_debugCam.rotation = vd;
}

void DebugState::enter()
{
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

		_debugLook.x -= deltaMouse.x / 100.0f;
		_debugLook.y += deltaMouse.y / 100.0f;

		if (_debugLook.y > qpi)
			_debugLook.y = qpi;
		else if (_debugLook.y < -qpi)
			_debugLook.y = -qpi;

		_debugCam.rotation = glm::angleAxis(_debugLook.x, glm::vec3(0.f, 0.f, 1.f))
				* glm::angleAxis(_debugLook.y, glm::vec3(0.f, 1.f, 0.f));

		_debugCam.position += _debugCam.rotation * _movement * dt * (_sonicMode ? 100.f : 10.f);
	}
}

void DebugState::draw(GameRenderer* r)
{
	// Draw useful information like camera position.
	std::stringstream ss;
	ss << "Camera Position: " << glm::to_string(_debugCam.position);

	TextRenderer::TextInfo ti;
	ti.text = ss.str();
	ti.font = 2;
	ti.screenPosition = glm::vec2( 10.f, 10.f );
	ti.size = 15.f;
	ti.baseColour = glm::u8vec3(255);
	r->text.renderText(ti);

	State::draw(r);
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
		case sf::Keyboard::F:
			_freeLook = !_freeLook;
			break;
		case sf::Keyboard::LShift:
			_sonicMode = true;
			break;
		case sf::Keyboard::P:
			printCameraDetails();
			break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(e.key.code) {
		case sf::Keyboard::W:
		case sf::Keyboard::S:
			_movement.x = 0.f;
			break;
		case sf::Keyboard::A:
		case sf::Keyboard::D:
			_movement.y = 0.f;
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

void DebugState::printCameraDetails()
{
	std::cout << " " << _debugCam.position.x << " " << _debugCam.position.y << " " << _debugCam.position.z
			  << " " << _debugCam.rotation.x << " " << _debugCam.rotation.y << " " << _debugCam.rotation.z
			  << " " << _debugCam.rotation.w << std::endl;
}

void DebugState::spawnVehicle(unsigned int id)
{
	auto ch = game->getPlayer()->getCharacter();
	if(! ch) return;

	glm::vec3 fwd = ch->rotation * glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 hit, normal;
	if(game->hitWorldRay(ch->position + (fwd * 5.f), {0.f, 0.f, -2.f}, hit, normal)) {
		auto spawnpos = hit + normal;
		getWorld()->createVehicle(id, spawnpos, glm::quat());
	}
}

const ViewCamera &DebugState::getCamera()
{
	return _debugCam;
}
