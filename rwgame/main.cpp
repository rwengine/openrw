#define GLEW_STATIC
#include <GL/glew.h>

#include <engine/GameWorld.hpp>
#include <loaders/LoaderDFF.hpp>
#include <render/DebugDraw.hpp>
#include <render/Model.hpp>
#include <objects/GTACharacter.hpp>
#include <objects/GTAVehicle.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "menustate.hpp"
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
GTACharacter* player = nullptr;

DebugDraw* debugDrawer = nullptr;

bool inFocus = false;
int debugMode = 0;

sf::Font font;

glm::vec3 viewPosition;
glm::vec2 viewAngles;

void setViewParameters(const glm::vec3 &center, const glm::vec2 &angles)
{
	viewPosition = center;
	viewAngles = angles;
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

void setPlayerCharacter(GTACharacter *playerCharacter)
{
	player = playerCharacter;
}

GTACharacter* getPlayerCharacter()
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

// Commands.
std::map<std::string, std::function<void (std::string)>> Commands = {
	{"pedestrian-vehicle", 
		[&](std::string) {
			glm::vec3 hit, normal;
			if(hitWorldRay(hit, normal)) {
				auto ped = gta->createPedestrian(2, hit+glm::vec3(0.f,10.f,0.f));
				// Pick random vehicle.
				auto it = gta->vehicleTypes.begin();
				std::uniform_int_distribution<int> uniform(0, 9);
				for(size_t i = 0, n = uniform(gta->randomEngine); i != n; i++) {
					it++;
				}
				auto spawnpos = hit + normal;
				auto vehicle = gta->createVehicle(it->first, spawnpos, glm::quat(glm::vec3(0.f, 0.f, -viewAngles.x * PiOver180)));
				ped->enterVehicle(vehicle, 0);
			}
		}
	},
	{"vehicle-test", 
		[&](std::string) {
			glm::vec3 hit, normal;
			if(hitWorldRay(hit, normal)) {
				glm::vec3 spawnPos = hit + glm::vec3(-5, 0.f, 0.0) + normal;
				size_t k = 1;
				for(std::map<uint16_t, std::shared_ptr<VehicleData>>::iterator it = gta->vehicleTypes.begin();
					it != gta->vehicleTypes.end(); ++it) {
					if(it->first == 140) continue; // get this plane out of here.
					gta->createVehicle(it->first, spawnPos);
					spawnPos += glm::vec3(5, 0, 0);
					if((k++ % 4) == 0) { spawnPos += glm::vec3(-20, -15, 0); }
				}
			}
		}
	},
	{"pedestrian-test",
		[&](std::string) {
			glm::vec3 hit, normal;
			if(hitWorldRay(hit, normal)) {
				glm::vec3 spawnPos = hit + glm::vec3(-5, 0.f, 0.0) + normal;
				size_t k = 1;
				// Spawn every pedestrian.
				for(auto it = gta->pedestrianTypes.begin();
					it != gta->pedestrianTypes.end(); ++it) {
					gta->createPedestrian(it->first, spawnPos);
					spawnPos += glm::vec3(2.5, 0, 0);
					if((k++ % 6) == 0) { spawnPos += glm::vec3(-15, -2.5, 0); }
				}
			}
		}
	},
	{"list-ipl",
		[&](std::string) {
			for(std::map<std::string, std::string>::iterator it = gta->gameData.iplLocations.begin();
				it != gta->gameData.iplLocations.end();
				++it) {
				gta->logInfo(it->second);
			}
		}
	},
	{"load-ipl",
		[&](std::string line) {
			if(line.find(' ') != line.npos) {
				std::string ipl = line.substr(line.find(' ')+1);
				auto iplit = gta->gameData.iplLocations.find(ipl);
				if(iplit != gta->gameData.iplLocations.end()) {
					gta->logInfo("Loading: " + iplit->second);
					gta->loadZone(iplit->second);
					gta->placeItems(iplit->second);
				}
				else {
					gta->logInfo("Not found: " + ipl);
				}
			}
		}
	},
	/*{"",
		[&](std::string) {
			
		}
	}*/
};



void command(const std::string& line)
{
	std::string cmd;
	if(line.find(' ') != line.npos) {
		cmd = line.substr(0, line.find(' '));
	}
	else {
		cmd = line;
	}
	
	auto it = Commands.find(cmd);
	if(it != Commands.end()) {
		it->second(line);
	}
	else {
		gta->logInfo("Unkown command: " + cmd);
	}
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
	
	// Loade all of the IDEs.
	for(std::map<std::string, std::string>::iterator it = gta->gameData.ideLocations.begin();
		it != gta->gameData.ideLocations.end();
		++it) {
		gta->defineItems(it->second);
	}
	
	if(loadWorld) {
		// Load IPLs 
		for(std::map<std::string, std::string>::iterator it = gta->gameData.iplLocations.begin();
			it != gta->gameData.iplLocations.end();
			++it) {
			gta->loadZone(it->second);
			gta->placeItems(it->second);
		}
	}
	
    debugDrawer = new DebugDraw;
    debugDrawer->setShaderProgram(gta->renderer.worldProgram);
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    gta->dynamicsWorld->setDebugDrawer(debugDrawer);

	std::cout << "Loaded "
			  << gta->gameData.models.size() << " models, "
			  << gta->gameData.textures.size() << " textures" << std::endl;
}

void update(float dt)
{
	if (inFocus) {
		float qpi = glm::half_pi<float>();

		glm::mat4 view;
		view = glm::translate(view, viewPosition);
		view = glm::rotate(view, viewAngles.x, glm::vec3(0, 0, 1));
		view = glm::rotate(view, viewAngles.y - qpi, glm::vec3(1, 0, 0));
		view = glm::inverse(view);
		
		gta->gameTime += dt;

		gta->renderer.camera.worldPos = viewPosition;
		gta->renderer.camera.frustum.view = view;
		
		// Update all objects.
		for( size_t p = 0; p < gta->pedestrians.size(); ++p) {
			gta->pedestrians[p]->tick(dt);

			if(gta->pedestrians[p]->mHealth <= 0.f) {
				gta->destroyObject(gta->pedestrians[p]);
				p--;
			}
		}
		for( size_t v = 0; v < gta->vehicleInstances.size(); ++v ) {
			gta->vehicleInstances[v]->tick(dt);
			if(gta->vehicleInstances[v]->mHealth <= 0.f) {
				gta->destroyObject(gta->vehicleInstances[v]);
				v--;
			}
		}
		
		

		gta->dynamicsWorld->stepSimulation(dt, 2, dt);
	}
}

void render()
{
	// Update aspect ratio..
	gta->renderer.camera.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	//glEnable(GL_CULL_FACE);

    switch( debugMode ) {
    case 0:
        gta->renderer.renderWorld();
        break;

    case 1: {
		gta->renderer.renderWorld();
        glUseProgram(gta->renderer.worldProgram);
        glm::mat4 proj = gta->renderer.camera.frustum.projection();
        glm::mat4 view = gta->renderer.camera.frustum.view;
        glUniformMatrix4fv(gta->renderer.uniView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(gta->renderer.uniProj, 1, GL_FALSE, glm::value_ptr(proj));
        gta->renderer.renderPaths();
        break;
    }
    case 2: {
		gta->renderer.renderWorld();
        glUseProgram(gta->renderer.worldProgram);
        glm::mat4 proj = gta->renderer.camera.frustum.projection();
        glm::mat4 view = gta->renderer.camera.frustum.view;
        glUniformMatrix4fv(gta->renderer.uniView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(gta->renderer.uniProj, 1, GL_FALSE, glm::value_ptr(proj));
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

	MenuState* menuState = new MenuState();
	
	StateManager::get().enter(menuState);
	
	float accum = 0.f;
    float ts = 1.f / 60.f;
	
	// Loop until the window is closed or we run out of state.
	while (window.isOpen() && StateManager::get().states.size()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleGlobalEvent(event);
			handleCommandEvent(event);
			handleInputEvent(event);
			
			StateManager::get().states.back()->handleEvent(event);
		}

		accum += clock.restart().asSeconds();
		
		while ( accum >= ts ) {

			StateManager::get().tick(ts);

			update(ts);
			accum -= ts;
		}
		
		render();
		
		StateManager::get().draw(window);
		
		window.display();
	
	}

	return 0;
}
