#define GLEW_STATIC
#include <GL/glew.h>

#include <engine/GTAEngine.hpp>
#include <loaders/LoaderDFF.hpp>
#include <render/DebugDraw.hpp>
#include <ai/GTAAIController.hpp>
#include <ai/GTAPlayerAIController.hpp>
#include <objects/GTACharacter.hpp>
#include <objects/GTAVehicle.hpp>

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
float moveSpeed = 20.0f;
bool inFocus = false;
bool mouseGrabbed = true;
int debugMode = 0;

sf::Font font;

void handleEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Escape:
			window.close();
			break;
		case sf::Keyboard::Space:
			moveSpeed = 60.f;
			break;
		case sf::Keyboard::M:
			mouseGrabbed = ! mouseGrabbed;
			break;
		case sf::Keyboard::P:
            debugMode++;
            while(debugMode > 2) debugMode -= 3;
			break;
		default: break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
			case sf::Keyboard::Space:
				moveSpeed = 20.f;
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

void init(std::string gtapath)
{
	// GTA GET
	gta = new GTAEngine(gtapath);
	
	// This is harcoded in GTA III for some reason
	gta->gameData.loadIMG("/models/gta3");
	
	gta->load();
	
	// Test out a known IPL.
	/*gta->placeItems(gtapath + "/data/maps/industsw/industSW.ipl");
	gta->placeItems(gtapath + "/data/maps/industnw/industNW.ipl");
	gta->placeItems(gtapath + "/data/maps/industse/industSE.ipl");
	gta->placeItems(gtapath + "/data/maps/industne/industNE.ipl");*/
	
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

		glm::vec3 movement;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			movement.z = -1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			movement.z = 1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			movement.x = -1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			movement.x = 1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
			if( player == nullptr ) {
				playerCharacter = gta->createPedestrian(1, plyPos);
				player = new GTAPlayerAIController(playerCharacter);
			}
		}

		glm::mat4 view;
		view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.y, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.x, glm::vec3(0, 0, 1));

		if( player != nullptr ) {
			glm::quat playerCamera(glm::vec3(0.f, 0.f, -plyLook.x * PiOver180));
			movement.y = -movement.z;
			movement.z = 0.f;
			player->updateCameraDirection(playerCamera);
			player->updateMovementDirection(movement);
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
				plyPos += dt * moveSpeed * (glm::inverse(glm::mat3(view)) * movement);
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

		gta->dynamicsWorld->stepSimulation(dt, 1, dt);
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
	ss << fmod(floor(gta->gameTime), 24.f) << ":" << (floor(fmod(gta->gameTime, 1.f) * 60.f)) << " (" << gta->gameTime << ")";
	sf::Text text(ss.str(), font, 15);
	text.setPosition(10, 10);
	window.draw(text);
	
	while( gta->log.size() > 0 && gta->log.front().time + 10.f < gta->gameTime ) {
		gta->log.pop_front();
	}
	
	sf::Vector2f tpos(10.f, 40.f);
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
	
	size_t w = WIDTH, h = HEIGHT;
	int c;
	while( (c = getopt(argc, argv, "w:h:")) != -1) {
		switch(c) {
			case 'w':
				w = atoi(optarg);
				break;
			case 'h':
				h = atoi(optarg);
				break;
		}
	}

    sf::ContextSettings cs;
    cs.depthBits = 32;
    window.create(sf::VideoMode(w, h), "GTA3 Viewer", sf::Style::Close, cs);
	window.setVerticalSyncEnabled(true);

	init(argv[optind]);
	
	sf::Clock clock;

	float accum = 0.f;
    float ts = 1.f / 20.f;
	
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(event);
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
