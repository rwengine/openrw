#define GLEW_STATIC
#include <GL/glew.h>

#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>

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

LoaderDFF dffLoader;
GTAEngine* gta = nullptr;

glm::vec3 plyPos;
glm::vec2 plyLook;
float moveSpeed = 20.0f;
bool inFocus = false;
bool mouseGrabbed = true;

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
		default: break;
		}
		break;
	case sf::Event::KeyReleased:
		switch(event.key.code) {
			case sf::Keyboard::Space:
				moveSpeed = 20.f;
				break;
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
	glEnable(GL_DEPTH_TEST);
	
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
	
	plyPos = gta->itemCentroid / (float) gta->itemCount + glm::vec3(0, 0, 2);
	
	glm::vec3 spawnPos = plyPos + glm::vec3(-5, -20, 0);
	size_t k = 1;
	// Spawn every vehicle, cause why not.
	for(std::map<uint16_t, std::shared_ptr<LoaderIDE::CARS_t>>::iterator it = gta->vehicleTypes.begin();
		it != gta->vehicleTypes.end(); ++it) {
		gta->createVehicle(it->first, spawnPos);
		spawnPos += glm::vec3(5, 0, 0);
		if((k++ % 4) == 0) { spawnPos += glm::vec3(-20, -15, 0); }
	}
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

		glm::mat4 view;
		view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.y, glm::vec3(1, 0, 0));
		view = glm::rotate(view, plyLook.x, glm::vec3(0, 0, 1));

		if (glm::length(movement) > 0.f) {
			plyPos += dt * moveSpeed * (glm::inverse(glm::mat3(view)) * movement);
		}

		view = glm::translate(view, -plyPos);
		
		gta->gameTime += dt;

		gta->renderer.camera.worldPos = plyPos;
		gta->renderer.camera.frustum.view = view;
	}
}

void render()
{
	// Update aspect ratio..
	gta->renderer.camera.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	glEnable(GL_CULL_FACE);

	gta->renderer.renderWorld(gta);
	
	glUseProgram(0);
	window.pushGLStates();
	window.resetGLStates();
	std::stringstream ss;
	ss << floor(gta->gameTime) << " (Hour " << fmod(floor(gta->gameTime), 24.f) << ")";
	sf::Text t(ss.str(), font, 20);
	t.setPosition(10, 10);
	window.draw(t);
	window.popGLStates();
	
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
	
	if(! font.loadFromFile("DejaVuSansCondensed.ttf")) {
		std::cerr << "Failed to load font DejaVuSansCondensed" << std::endl;
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

	window.create(sf::VideoMode(w, h), "GTA3 Viewer", sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	init(argv[optind]);
	
	sf::Clock clock;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(event);
		}

		update(clock.restart().asSeconds());
		
		render();
		window.display();
		
	}

	return 0;
}
