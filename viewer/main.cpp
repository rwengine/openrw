#define GLEW_STATIC
#include <GL/glew.h>

#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/render/DebugDraw.hpp>
#include <renderwure/ai/GTAAIController.hpp>
#include <renderwure/ai/GTAPlayerAIController.hpp>
#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/objects/GTAVehicle.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

#include <memory>
#include <sstream>
#include <getopt.h>

constexpr int WIDTH  = 800,
              HEIGHT = 600;

constexpr double PiOver180 = 3.1415926535897932384626433832795028/180;

sf::RenderWindow window;

GTAEngine* gta = nullptr;

GTAPlayerAIController* player = nullptr;
GTACharacter* playerCharacter = nullptr;

DebugDraw* debugDrawer = nullptr;

glm::vec3 plyPos;
glm::vec2 plyLook;
glm::vec3 movement;
float moveSpeed = 20.0f;
bool inFocus = false;
bool mouseGrabbed = true;
int debugMode = 0;

sf::Font font;

bool showControls = true;

bool hitWorldRay(glm::vec3& hit, glm::vec3& normal)
{
	glm::mat4 view;
	view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
	view = glm::rotate(view, plyLook.y, glm::vec3(1, 0, 0));
	view = glm::rotate(view, plyLook.x, glm::vec3(0, 0, 1));
	glm::vec3 dir = glm::inverse(glm::mat3(view)) * glm::vec3(0.f, 0.f, 1.f) * -50.f;
	auto from = btVector3(plyPos.x, plyPos.y, plyPos.z);
	auto to = btVector3(plyPos.x+dir.x, plyPos.y+dir.y, plyPos.z+dir.z);
	btCollisionWorld::ClosestRayResultCallback ray(from, to);
	gta->dynamicsWorld->rayTest(from, to, ray);
	if( ray.hasHit() ) 
	{
		hit = glm::vec3(ray.m_hitPointWorld.x(), ray.m_hitPointWorld.y(),
						ray.m_hitPointWorld.z());
		normal = glm::vec3(ray.m_hitNormalWorld.x(), ray.m_hitNormalWorld.y(),
						   ray.m_hitNormalWorld.z());
		return true;
	}
	return false;
}

void command(const std::string& line)
{
	std::string cmd;
	if(line.find(' ') != line.npos) {
		cmd = line.substr(0, line.find(' '));
	}
	else {
		cmd = line;
	}
	
	if ("pedestrian-vehicle" == cmd) {
		glm::vec3 hit, normal;
		if(hitWorldRay(hit, normal)) {
			auto ped = gta->createPedestrian(2, plyPos+glm::vec3(0.f,10.f,0.f));
			// Pick random vehicle.
			auto it = gta->vehicleTypes.begin();
			std::uniform_int_distribution<int> uniform(0, 9);
			for(size_t i = 0, n = uniform(gta->randomEngine); i != n; i++) {
				it++;
			}
			auto spawnpos = hit + normal;
			auto vehicle = gta->createVehicle(it->first, spawnpos, glm::quat(glm::vec3(0.f, 0.f, -plyLook.x * PiOver180)));
			ped->setCurrentVehicle(vehicle);
		}
	}
	else if("player-vehicle" == cmd) {
		glm::vec3 hit, normal;
		if(hitWorldRay(hit, normal)) {
			playerCharacter = gta->createPedestrian(1, plyPos+glm::vec3(0.f,10.f,0.f));
			player = new GTAPlayerAIController(playerCharacter);

			// Pick random vehicle.
			auto it = gta->vehicleTypes.begin();
			std::uniform_int_distribution<int> uniform(0, 9);
			for(size_t i = 0, n = uniform(gta->randomEngine); i != n; i++) {
				it++;
			}
			
			auto spawnpos = hit + normal;
			auto vehicle = gta->createVehicle(it->first, spawnpos, glm::quat(glm::vec3(0.f, 0.f, -plyLook.x * PiOver180)));
			playerCharacter->setCurrentVehicle(vehicle);
		}
	}
	else if("player" == cmd) {
		playerCharacter = gta->createPedestrian(1, plyPos);
		player = new GTAPlayerAIController(playerCharacter);
	}
	else if("knock-down" == cmd) {
		for(auto it = gta->pedestrians.begin(); it != gta->pedestrians.end(); ++it) {
			(*it)->changeAction(GTACharacter::KnockedDown);
		}
	}
	else if("list-ipl" == cmd) {
		for(std::map<std::string, std::string>::iterator it = gta->gameData.iplLocations.begin();
			it != gta->gameData.iplLocations.end();
			++it) {
			gta->logInfo(it->second);
		}
	}
	else if("load-ipl" == cmd) {
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
	else if("create-instance" == cmd) {
		if(line.find(' ') != line.npos) {
			std::string ID = line.substr(line.find(' ')+1);
			int intID = atoi(ID.c_str());
			auto archit = gta->objectTypes.find(intID);
			if(archit != gta->objectTypes.end()) {
				gta->createInstance(archit->first, plyPos);
			}
			else {
				gta->logInfo("Unkown Object: " + ID);
			}
		}
	}
	else {
		gta->logInfo("Unkown command: " + cmd);
	}
}

void handleGlobalEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Escape:
			window.close();
			break;
		case sf::Keyboard::M:
			//commandMode = !commandMode;
			command("player-vehicle");
			break;
		default: break;
		}
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
		case sf::Keyboard::Space:
			moveSpeed = 60.f;
			break;
		case sf::Keyboard::M:
			mouseGrabbed = ! mouseGrabbed;
			break;
		case sf::Keyboard::P:
            debugMode+=1;
            while(debugMode > 2) debugMode -= 3;
			break;
		case sf::Keyboard::W:
			movement.y = -1;
			break;
		case sf::Keyboard::S:
			movement.y = 1;
			break;
		case sf::Keyboard::A:
			movement.x = -1;
			break;
		case sf::Keyboard::D:
			movement.x = 1;
			break;
		default: break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
		case sf::Keyboard::Space:
				moveSpeed = 20.f;
				break;
		case sf::Keyboard::W:
			movement.y = 0;
			break;
		case sf::Keyboard::S:
			movement.y = 0;
			break;
		case sf::Keyboard::A:
			movement.x = 0;
			break;
		case sf::Keyboard::D:
			movement.x = 0;
			break;
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
		case sf::Keyboard::F1:
			showControls = !showControls;
			break;
		case sf::Keyboard::F2:
			command("pedestrian-vehicle");
			break;
		case sf::Keyboard::F3:
			command("player-vehicle");
			break;
		}
		break;
	}
}

void init(std::string gtapath, bool loadWorld)
{
	// GTA GET
	gta = new GTAEngine(gtapath);
	
	// This is harcoded in GTA III for some reason
	gta->gameData.loadIMG("/models/gta3");
	
	gta->load();
	
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
	
	glm::vec3 spawnPos = plyPos + glm::vec3(-5, -20, 0.0);
	size_t k = 1;
	// Spawn every vehicle, cause why not.
	for(std::map<uint16_t, std::shared_ptr<CarData>>::iterator it = gta->vehicleTypes.begin();
		it != gta->vehicleTypes.end(); ++it) {
		if(it->first == 140) continue; // get this plane out of here.
		gta->createVehicle(it->first, spawnPos);
		spawnPos += glm::vec3(5, 0, 0);
        if((k++ % 4) == 0) { spawnPos += glm::vec3(-20, -15, 0); }
	}

	spawnPos = plyPos + glm::vec3(-5, 20 + (2.5 * gta->pedestrianTypes.size()/4), 0);
    k = 1;
    // Spawn every pedestrian.
    /*for(auto it = gta->pedestrianTypes.begin();
        it != gta->pedestrianTypes.end(); ++it) {
        gta->createPedestrian(it->first, spawnPos);
        spawnPos += glm::vec3(2.5, 0, 0);
        if((k++ % 6) == 0) { spawnPos += glm::vec3(-15, -2.5, 0); }
    }*/
	
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
		if (mouseGrabbed) {
			sf::Vector2i screenCenter{sf::Vector2i{window.getSize()} / 2};
			sf::Vector2i mousePos = sf::Mouse::getPosition(window);
			sf::Vector2i deltaMouse = mousePos - screenCenter;
			sf::Mouse::setPosition(screenCenter, window);

			plyLook.x += deltaMouse.x / 10.0;
			plyLook.y += deltaMouse.y / 10.0;

			if (plyLook.y > 90)
				plyLook.y = 90;
			else if (plyLook.y < -90)
				plyLook.y = -90;
		}

		glm::mat4 view;
		view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.y, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.x, glm::vec3(0, 0, 1));
		
		if( player != nullptr ) {
			glm::quat playerCamera(glm::vec3(0.f, 0.f, -plyLook.x * PiOver180));
			player->updateCameraDirection(playerCamera);
			player->updateMovementDirection(glm::vec3(movement.x, -movement.y, movement.z));
			player->setRunning(moveSpeed > 21.f);

			float viewDistance = playerCharacter->getCurrentVehicle() ? -3.5f : -2.5f;
			glm::vec3 localView = glm::inverse(glm::mat3(view)) * glm::vec3(0.f, -0.5f, viewDistance);
			if(playerCharacter->getCurrentVehicle()) {
				plyPos = playerCharacter->getCurrentVehicle()->getPosition();
			}
			else {
				plyPos = playerCharacter->getPosition();
			}
			view = glm::translate(view, -plyPos + localView);
		}
		else {
			if (glm::length(movement) > 0.f) {
				plyPos += dt * moveSpeed * (glm::inverse(glm::mat3(view)) * glm::vec3(movement.x, movement.z, movement.y));
			}
			view = glm::translate(view, -plyPos);
		}
		
		gta->gameTime += dt;

		gta->renderer.camera.worldPos = plyPos;
		gta->renderer.camera.frustum.view = view;

		// TODO: move this inside the engine
        for( size_t p = 0; p < gta->pedestrians.size(); ++p ) {
			gta->pedestrians[p]->tick(dt);
        }
		for( size_t v = 0; v < gta->vehicleInstances.size(); ++v ) {
			gta->vehicleInstances[v]->tick(dt);
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
	ss << "Time: " << fmod(floor(gta->gameTime), 24.f) << ":" << (floor(fmod(gta->gameTime, 1.f) * 60.f)) << std::endl;
	ss << "Game Time: " << gta->gameTime << std::endl;
	ss << "Camera: " << plyPos.x << " " << plyPos.y << " " << plyPos.z << std::endl; 
	sf::Text text(ss.str(), font, 15);
	text.setPosition(10, 10);
	window.draw(text);
	
	if(showControls) {
		std::stringstream ss;
		ss << "F1 - Toggle Help" << std::endl;
		ss << "F2 - Create Vehicle (with driver)" << std::endl;
		ss << "F3 - Create Vehicle (with player)" << std::endl;
		text.setString(ss.str());
		text.setPosition(10, 100);
		window.draw(text);
	}
	
	while( gta->log.size() > 0 && gta->log.front().time + 10.f < gta->gameTime ) {
		gta->log.pop_front();
	}
	
	sf::Vector2f tpos(10.f, 150.f);
	text.setCharacterSize(15);
	for(auto it = gta->log.begin(); it != gta->log.end(); ++it) {
		text.setString(it->message);
		switch(it->type) {
		case GTAEngine::LogEntry::Error:
			text.setColor(sf::Color::Red);
			break;
		case GTAEngine::LogEntry::Warning:
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
		tpos.y += text.getLocalBounds().height;
	}
	
	static size_t fc = 0;
	if(fc++ == 60) 
	{
		std::cout << "Rendered: " << gta->renderer.rendered << " / Culled: " << gta->renderer.culled << std::endl;
		fc = 0;
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <path to GTA3 root folder>" << std::endl;
		exit(1);
	}
	
	if(! font.loadFromFile("DejaVuSansMono.ttf")) {
		std::cerr << "Failed to load font" << std::endl;
	}

	glewExperimental = GL_TRUE;
	glewInit();

	bool loadWorld = false;
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
				loadWorld = true;
				break;
		}
	}

    sf::ContextSettings cs;
    cs.depthBits = 32;
    window.create(sf::VideoMode(w, h), "GTA3 Viewer", sf::Style::Close, cs);
	window.setVerticalSyncEnabled(true);

	init(argv[optind], loadWorld);
	
	sf::Clock clock;

	float accum = 0.f;
    float ts = 1.f / 60.f;
	
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleGlobalEvent(event);
			handleCommandEvent(event);
			handleInputEvent(event);
		}

		accum += clock.restart().asSeconds();
		
		while ( accum >= ts ) {
			update(ts);
			accum -= ts;
		}
		
		render();
		window.display();
	
	}

	return 0;
}
