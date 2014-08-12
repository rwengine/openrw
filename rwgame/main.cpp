#define GLEW_STATIC
#include <GL/glew.h>

#include <engine/GameWorld.hpp>
#include <loaders/LoaderDFF.hpp>
#include <render/DebugDraw.hpp>
#include <render/Model.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <ai/PlayerController.hpp>

#include <script/ScriptMachine.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <data/CutsceneData.hpp>

#include "loadingstate.hpp"
#include <SFML/Graphics.hpp>

#include <memory>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include "game.hpp"

#define ENV_GAME_PATH_NAME ("OPENRW_GAME_PATH")
#define GAME_TIMESTEP (1.f/30.f)

constexpr int WIDTH  = 800,
              HEIGHT = 600;

sf::RenderWindow window;

GameWorld* gta = nullptr;

DebugDraw* debugDrawer = nullptr;

bool inFocus = true;
int debugMode = 0;
float accum = 0.f;

// Camera poisitions to interpolate between ticks.
ViewCamera lastCam, nextCam;

sf::Font font;

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

void skipTime(float time)
{
	accum += time;
}

bool hitWorldRay(glm::vec3 &hit, glm::vec3 &normal, GameObject** object)
{
	const ViewCamera& vc = StateManager::get().states.back()->getCamera();
	btVector3 from(vc.position.x, vc.position.y, vc.position.z);
	glm::vec3 tmp = vc.rotation * glm::vec3(1000.f, 0.f, 0.f);
	btVector3 to(from.x() + tmp.x, from.y() + tmp.y, from.x() + tmp.z);

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
			gta->state.minute -= 30.f;
			break;
		case sf::Keyboard::RBracket:
			gta->state.minute += 30.f;
			break;
		break;
		default: break;
		}
		default: break;
	}
}

void init(std::string gtapath)
{
	// GTA GET
	gta = new GameWorld(gtapath);
	
	// This is harcoded in GTA III for some reason
	gta->gameData.loadIMG("/models/gta3");
	gta->gameData.loadIMG("/models/txd");
	gta->gameData.loadIMG("/anim/cuts");

	gta->load();
	
	// Load dynamic object data
	gta->gameData.loadDynamicObjects(gtapath + "/data/object.dat");

	gta->gameData.loadGXT("english.gxt");

	gta->gameTime = 0.f;
	
	debugDrawer = new DebugDraw;
	debugDrawer->setShaderProgram(gta->renderer.worldProgram);
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	gta->dynamicsWorld->setDebugDrawer(debugDrawer);

	//setViewParameters( { -260.f, -151.5f, 9.f }, { -0.3f, 0.05f } );

	std::cout << "Loaded "
			  << gta->gameData.models.size() << " models, "
			  << gta->gameData.textures.size() << " textures" << std::endl;
}

void update(float dt)
{
	gta->_work->update();

	static float clockAccumulator = 0.f;
	if (inFocus) {
		gta->gameTime += dt;

		clockAccumulator += dt;
		while( clockAccumulator >= 1.f ) {
			gta->state.minute ++;
			if( gta->state.minute >= 60 ) {
				gta->state.minute = 0;
				gta->state.hour ++;
				if( gta->state.hour >= 24 ) {
					gta->state.hour = 0;
				}
			}
			clockAccumulator -= 1.f;
		}

		for( GameObject* object : gta->objects ) {
			object->_updateLastTransform();
			object->tick(dt);
		}

		gta->destroyQueuedObjects();
		gta->state.texts.clear();

		gta->dynamicsWorld->stepSimulation(dt, 2, dt);

		if( getWorld()->script ) {
			try {
				getWorld()->script->execute(dt);
			}
			catch( SCMException& ex ) {
				std::cerr << ex.what() << std::endl;
				getWorld()->logError( ex.what() );
				throw;
			}
		}
	}

	// render() needs two cameras to smoothly interpolate between ticks.
	lastCam = nextCam;
	nextCam	= StateManager::get().states.back()->getCamera();
}

void render(float alpha)
{
	ViewCamera viewCam;
	if( gta->state.currentCutscene != nullptr && gta->state.cutsceneStartTime >= 0.f )
	{
		auto cutscene = gta->state.currentCutscene;
		float cutsceneTime = std::min(gta->gameTime - gta->state.cutsceneStartTime,
									  cutscene->tracks.duration);
		cutsceneTime += GAME_TIMESTEP * alpha;
		glm::vec3 cameraPos = cutscene->tracks.getPositionAt(cutsceneTime),
				targetPos = cutscene->tracks.getTargetAt(cutsceneTime);
		float zoom = cutscene->tracks.getZoomAt(cutsceneTime);
		viewCam.frustum.fov = glm::radians(zoom);
		float tilt = cutscene->tracks.getRotationAt(cutsceneTime);

		auto direction = glm::normalize(targetPos - cameraPos);
		auto right = glm::normalize(glm::cross(glm::vec3(0.f, 0.f, 1.f), direction));
		auto up = glm::normalize(glm::cross(direction, right));

		glm::mat3 m;
		m[0][0] = direction.x;
		m[0][1] = right.x;
		m[0][2] = up.x;

		m[1][0] = direction.y;
		m[1][1] = right.y;
		m[1][2] = up.y;

		m[2][0] = direction.z;
		m[2][1] = right.z;
		m[2][2] = up.z;

		auto qtilt = glm::angleAxis(glm::radians(tilt), direction);

		cameraPos += cutscene->meta.sceneOffset;
		targetPos += cutscene->meta.sceneOffset;

		viewCam.position = cameraPos;
		viewCam.rotation = glm::inverse(glm::quat_cast(m)) * qtilt;
	}
	else
	{
		// There's no cutscene playing - use the camera returned by the State.
		viewCam.position = glm::mix(lastCam.position, nextCam.position, alpha);
		viewCam.rotation = glm::slerp(lastCam.rotation, nextCam.rotation, alpha);
	}

	viewCam.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	gta->renderer.renderWorld(viewCam, alpha);

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
		<< ":" << std::setw(2) << gta->getMinute() << " (" << gta->gameTime << "s)\n";
	ss << "View: " << viewCam.position.x << " " << viewCam.position.y << " " << viewCam.position.z << "\n";
	ss << "Drawn " << gta->renderer.rendered << " / " << gta->renderer.culled << " Culled " << " " << gta->renderer.frames << "	" << gta->renderer.geoms << "\n";
	if( gta->state.player ) {
		ss << "Activity: ";
		if( gta->state.player->getCurrentActivity() ) {
			ss << gta->state.player->getCurrentActivity()->name();
		}
		else {
			ss << "Idle";
		}
		ss << std::endl;
	}
	
	sf::Text text(ss.str(), font, 14);
	text.setPosition(10, 10);
	window.draw(text);
	
	while( gta->log.size() > 0 && gta->log.front().time + 10.f < gta->gameTime ) {
		gta->log.pop_front();
	}
	
	sf::Vector2f tpos(10.f, window.getSize().y - 30.f);
	text.setCharacterSize(14);
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

	/// @todo this should be done by GameRenderer? but it doesn't have any font support yet
	if( gta->gameTime < gta->state.osTextStart + gta->state.osTextTime ) {
		sf::Text messageText(gta->state.osTextString, font, 15);
		auto sz = window.getSize();

		auto b = messageText.getLocalBounds();
		float lowerBar = sz.y - sz.y * 0.1f;
		messageText.setPosition(sz.x / 2.f - std::round(b.width / 2.f), lowerBar - std::round(b.height / 2.f));
		window.draw(messageText);
	}

	for(auto& t : gta->state.texts) {
		sf::Text messageText(t.text, font, 15);

		glm::vec2 scpos(t.position.x, t.position.y);
		auto s = window.getSize();
		scpos /= glm::vec2(640.f, 480.f);
		scpos *= glm::vec2(s.x, s.y);

		messageText.setPosition(scpos.x, scpos.y);

		window.draw(messageText);
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
    window.create(sf::VideoMode(w, h), "", sf::Style::Default, cs);
	window.setVerticalSyncEnabled(true);
	window.setMouseCursorVisible(false);

	init(getGamePath());
	
	sf::Clock clock;

	StateManager::get().enter(new LoadingState);

	float ts = GAME_TIMESTEP;
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

	delete gta;

	return 0;
}
