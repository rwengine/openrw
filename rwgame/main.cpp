#define GLEW_STATIC
#include <GL/glew.h>

#include <engine/GameWorld.hpp>
#include <loaders/LoaderDFF.hpp>
#include <render/DebugDraw.hpp>
#include <render/Model.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <ai/CharacterController.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "loadingstate.hpp"
#include <SFML/Graphics.hpp>

#include <memory>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include "game.hpp"

#define ENV_GAME_PATH_NAME ("OPENRW_GAME_PATH")

constexpr int WIDTH  = 800,
              HEIGHT = 600;

sf::RenderWindow window;

GameWorld* gta = nullptr;
CharacterObject* player = nullptr;

DebugDraw* debugDrawer = nullptr;

bool inFocus = false;
int debugMode = 0;

sf::Font font;

glm::vec3 viewPosition { -260.f, -151.5f, 9.f }, lastViewPosition;
glm::vec2 viewAngles { -0.3f, 0.05f }, lastViewAngles;

void setViewParameters(const glm::vec3 &center, const glm::vec2 &angles)
{
	lastViewPosition = viewPosition;
	lastViewAngles = viewAngles;
	viewPosition = center;
	viewAngles = angles;
}

glm::vec3& getViewPosition()
{
	return viewPosition;
}

glm::vec2& getViewAngles()
{
	return viewAngles;
}

sf::Window& getWindow()
{
	return window;
}

GameWorld* getWorld()
{
	return gta;
}

sf::Font& getFont()
{
	return font;
}

void setPlayerCharacter(CharacterObject *playerCharacter)
{
	player = playerCharacter;
}

CharacterObject* getPlayerCharacter()
{
	return player;
}

bool hitWorldRay(glm::vec3 &hit, glm::vec3 &normal, GameObject** object)
{
	glm::mat4 view;
	view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
	view = glm::rotate(view, viewAngles.y, glm::vec3(1, 0, 0));
	view = glm::rotate(view, viewAngles.x, glm::vec3(0, 0, 1));
	glm::vec3 dir = glm::inverse(glm::mat3(view)) * glm::vec3(0.f, 0.f, 1.f) * -50.f;
	auto from = btVector3(viewPosition.x, viewPosition.y, viewPosition.z);
	auto to = btVector3(viewPosition.x+dir.x, viewPosition.y+dir.y, viewPosition.z+dir.z);
	btCollisionWorld::ClosestRayResultCallback ray(from, to);
	gta->dynamicsWorld->rayTest(from, to, ray);
	if( ray.hasHit() ) 
	{
		hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
						ray.m_hitPointWorld.z());
		normal = glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
						   ray.m_hitNormalWorld.z());
		if(object) {
			*object = static_cast<GameObject*>(ray.m_collisionObject->getUserPointer());
		}
		return true;
	}
	return false;
}

bool hitWorldRay(const glm::vec3 &start, const glm::vec3 &direction, glm::vec3 &hit, glm::vec3 &normal, GameObject **object)
{
	auto from = btVector3(start.x, start.y, start.z);
	auto to = btVector3(start.x+direction.x, start.y+direction.y, start.z+direction.z);
	btCollisionWorld::ClosestRayResultCallback ray(from, to);
	gta->dynamicsWorld->rayTest(from, to, ray);
	if( ray.hasHit() )
	{
		hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
						ray.m_hitPointWorld.z());
		normal = glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
						   ray.m_hitNormalWorld.z());
		if(object) {
			*object = static_cast<GameObject*>(ray.m_collisionObject->getUserPointer());
		}
		return true;
	}
	return false;
}

void handleGlobalEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::KeyPressed:
		break;
	case sf::Event::GainedFocus:
		inFocus = true;
		break;
	case sf::Event::LostFocus:
		inFocus = false;
		break;
	default: break;
	}
}

void handleInputEvent(sf::Event &event)
{
	switch(event.type) {
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::P:
            debugMode+=1;
            while(debugMode > 2) debugMode -= 3;
			break;
		default: break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
		default: break;
		}
		break;
	default: break;
	}
}

void handleCommandEvent(sf::Event &event)
{
	switch(event.type) {
		case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::LBracket:
			gta->gameTime -= 60.f;
			break;
		case sf::Keyboard::RBracket:
			gta->gameTime += 60.f;
			break;
		break;
		default: break;
		}
		default: break;
	}
}

void init(std::string gtapath, bool loadWorld)
{
	// GTA GET
	gta = new GameWorld(gtapath);
	
	// This is harcoded in GTA III for some reason
	gta->gameData.loadIMG("/models/gta3");
	gta->gameData.loadIMG("/models/txd");
	
	gta->load();
	
	// Load dynamic object data
	gta->gameData.loadDynamicObjects(gtapath + "/data/object.dat");
	
	// Set time to noon.
	gta->gameTime = 12.f * 60.f;
	
    debugDrawer = new DebugDraw;
    debugDrawer->setShaderProgram(gta->renderer.worldProgram);
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    gta->dynamicsWorld->setDebugDrawer(debugDrawer);

	setViewParameters( { -260.f, -151.5f, 9.f }, { -0.3f, 0.05f } );

	std::cout << "Loaded "
			  << gta->gameData.models.size() << " models, "
			  << gta->gameData.textures.size() << " textures" << std::endl;
}

void update(float dt)
{
	if (inFocus) {
		gta->gameTime += dt;

		for( GameObject* object : gta->objects ) {
			object->_updateLastTransform();
			object->tick(dt);
		}

		gta->destroyQueuedObjects();

		gta->dynamicsWorld->stepSimulation(dt, 2, dt);
	}
}

void render(float alpha)
{
	gta->_work->update();

	float qpi = glm::half_pi<float>();

	glm::mat4 view;
	view = glm::translate(view, glm::mix(lastViewPosition, viewPosition, alpha));
	auto va = glm::mix(lastViewAngles, viewAngles, alpha);
	view = glm::rotate(view, va.x, glm::vec3(0, 0, 1));
	view = glm::rotate(view, va.y - qpi, glm::vec3(1, 0, 0));
	view = glm::inverse(view);

	gta->renderer.camera.worldPos = viewPosition;
	gta->renderer.camera.frustum.view = view;

	// Update aspect ratio..
	gta->renderer.camera.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	//glEnable(GL_CULL_FACE);

	gta->renderer.renderWorld(alpha);

    switch( debugMode ) {
	case 0: break;

	case 1: {
        glUseProgram(gta->renderer.worldProgram);
		gta->renderer.uploadUBO<ObjectUniformData>(
					gta->renderer.uboObject, {
						glm::mat4(),
						glm::vec4(1.f),
						1.f, 1.f
					});
		gta->renderer.renderPaths();
        break;
    }
	case 2: {
		glUseProgram(gta->renderer.worldProgram);
		gta->renderer.uploadUBO<ObjectUniformData>(
					gta->renderer.uboObject, {
						glm::mat4(),
						glm::vec4(1.f),
						1.f, 1.f
					});
		gta->dynamicsWorld->debugDrawWorld();
        debugDrawer->drawAllLines();

        break;
    }
    }

	window.resetGLStates();
	
	std::stringstream ss;
	ss << std::setfill('0') << "Time: " << std::setw(2) << gta->getHour() 
		<< ":" << std::setw(2) << gta->getMinute() << std::endl;
	ss << "Game Time: " << gta->gameTime << std::endl;
	ss << "Camera: " << viewPosition.x << " " << viewPosition.y << " " << viewPosition.z << std::endl;
	ss << "Renderered " << gta->renderer.rendered << " / " << gta->renderer.culled << std::endl;
	if( player ) {
		ss << "Activity: ";
		if( player->controller->getCurrentActivity() ) {
			ss << player->controller->getCurrentActivity()->name();
		}
		else {
			ss << "Idle";
		}
		ss << std::endl;
	}
	
	sf::Text text(ss.str(), font, 15);
	text.setPosition(10, 10);
	window.draw(text);
	
	while( gta->log.size() > 0 && gta->log.front().time + 10.f < gta->gameTime ) {
		gta->log.pop_front();
	}
	
	sf::Vector2f tpos(10.f, window.getSize().y - 30.f);
	text.setCharacterSize(15);
	for(auto it = gta->log.begin(); it != gta->log.end(); ++it) {
		text.setString(it->message);
		switch(it->type) {
		case GameWorld::LogEntry::Error:
			text.setColor(sf::Color::Red);
			break;
		case GameWorld::LogEntry::Warning:
			text.setColor(sf::Color::Yellow);
			break;
		default:
			text.setColor(sf::Color::White);
			break;
		}
		
		// Interpolate the color
		auto c = text.getColor();
		c.a = (gta->gameTime - it->time > 5.f) ? 255 - (((gta->gameTime - it->time) - 5.f)/5.f) * 255 : 255;
		text.setColor(c);
		
		text.setPosition(tpos);
		window.draw(text);
		tpos.y -= text.getLocalBounds().height;
	}
}

std::string getGamePath()
{
	auto v = getenv(ENV_GAME_PATH_NAME);
	return v ? v : "";
}

int main(int argc, char *argv[])
{
	if(! font.loadFromFile(getGamePath() + "/DejaVuSansMono.ttf")) {
		std::cerr << "Failed to load font" << std::endl;
	}

	glewExperimental = GL_TRUE;
	glewInit();

	bool loadWorld = true;
	size_t w = WIDTH, h = HEIGHT;
	int c;
	while( (c = getopt(argc, argv, "w:h:l")) != -1) {
		switch(c) {
			case 'w':
				w = atoi(optarg);
				break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'l':
				loadWorld = false;
				break;
		}
	}

    sf::ContextSettings cs;
    cs.depthBits = 32;
    window.create(sf::VideoMode(w, h), "", sf::Style::Default, cs);
	window.setVerticalSyncEnabled(true);
	window.setMouseCursorVisible(false);

	init(getGamePath(), loadWorld);
	
	sf::Clock clock;

	StateManager::get().enter(new LoadingState);
	
	float accum = 0.f;
	float ts = 1.f / 60.f;
	float timescale = 1.f;
	
	// Loop until the window is closed or we run out of state.
	while (window.isOpen() && StateManager::get().states.size()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleGlobalEvent(event);
			handleCommandEvent(event);
			handleInputEvent(event);
			
			StateManager::get().states.back()->handleEvent(event);
		}

		accum += clock.restart().asSeconds() * timescale;
		
		while ( accum >= ts ) {

			StateManager::get().tick(ts);

			update(ts);
			accum -= ts;
		}

		float alpha = accum / ts;
		
		render(alpha);
		
		StateManager::get().draw(window);
		
		window.display();
	
	}

	return 0;
}
