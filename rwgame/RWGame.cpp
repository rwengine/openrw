#include "RWGame.hpp"
#include "State.hpp"
#include "loadingstate.hpp"
#include "DrawUI.hpp"
#include "ingamestate.hpp"
#include "menustate.hpp"

#include <engine/GameObject.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>
#include <render/DebugDraw.hpp>
#include <script/ScriptMachine.hpp>

#include <data/CutsceneData.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

DebugDraw* debug;

StdOutReciever logPrinter;

RWGame::RWGame(const std::string& gamepath, int argc, char* argv[])
	: engine(nullptr), renderer(nullptr), inFocus(true), showDebugStats(false),
	  accum(0.f), timescale(1.f)
{
	size_t w = GAME_WINDOW_WIDTH, h = GAME_WINDOW_HEIGHT;
	bool fullscreen = false;
	bool newgame = false;

	for( int i = 1; i < argc; ++i )
	{
		if( strcasecmp( "-w", argv[i] ) == 0 && i+1 < argc )
		{
			w = std::atoi(argv[i+1]);
		}
		if( strcasecmp( "-h", argv[i] ) == 0 && i+1 < argc )
		{
			h = std::atoi(argv[i+1]);
		}
		if( strcasecmp( "-f", argv[i] ) == 0 )
		{
			fullscreen = true;
		}
		if( strcmp( "--newgame", argv[i] ) == 0 )
		{
			newgame = true;
		}
	}
	
	
	sf::Uint32 style = sf::Style::Default;
	if( fullscreen )
	{
		style |= sf::Style::Fullscreen;
	}

	sf::ContextSettings cs;
	cs.depthBits = 32;
	window.create(sf::VideoMode(w, h), "",  style, cs);
	window.setVerticalSyncEnabled(true);
	window.setMouseCursorVisible(false);

	glewExperimental = GL_TRUE;
	glewInit();

	engine = new GameWorld(gamepath);
	engine->logger.addReciever(&logPrinter);
	
	// Initalize all the archives.
	engine->gameData.loadIMG("/models/gta3");
	//engine->gameData.loadIMG("/models/txd");
	engine->gameData.loadIMG("/anim/cuts");
	
	// Initialize renderer
	renderer = new GameRenderer(engine);
	
	// Set up text renderer
	renderer->text.setFontTexture(0, "pager");
	renderer->text.setFontTexture(1, "font1");
	renderer->text.setFontTexture(2, "font2");

	/// @TODO expand this here.
	engine->load();
	debug = new DebugDraw;
	debug->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawConstraintLimits);
	engine->dynamicsWorld->setDebugDrawer(debug);

	engine->gameData.loadDynamicObjects(gamepath + "/data/object.dat");

	/// @TODO language choices.
	engine->gameData.loadGXT("english.gxt");
	
	getRenderer()->water.setWaterTable(engine->gameData.waterHeights, 48, engine->gameData.realWater, 128*128);
	
	for(int m = 0; m < MAP_BLOCK_SIZE; ++m)
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		engine->gameData.loadTXD(name + ".txd");
	}

	auto loading = new LoadingState(this);
	if( newgame )
	{
		loading->setNextState(new IngameState(this));
	}
	else
	{
		loading->setNextState(new MenuState(this));
	}
	
	StateManager::get().enter(loading);

	engine->logger.info("Game", "Started");
}

RWGame::~RWGame()
{
	delete renderer;
	delete engine;
}

int RWGame::run()
{
	clock.restart();

	// Loop until the window is closed or we run out of state.
	while (window.isOpen() && StateManager::get().states.size()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::GainedFocus:
				inFocus = true;
				break;
			case sf::Event::LostFocus:
				inFocus = false;
				break;
			case sf::Event::KeyPressed:
				globalKeyEvent(event);
				break;
			case sf::Event::Closed:
				return 0;
			default: break;
			}

			StateManager::get().states.back()->handleEvent(event);
		}
		
		if(! window.isOpen() )
		{
			break;
		}

		float timer = clock.restart().asSeconds();
		accum += timer * timescale;

		while ( accum >= GAME_TIMESTEP ) {

			if( ! getWorld()->isPaused() )
			{
				StateManager::get().tick(GAME_TIMESTEP);

				tick(GAME_TIMESTEP);
			}
			
			accum -= GAME_TIMESTEP;
			
			// Throw away time if the accumulator reaches too high.
			if ( accum > GAME_TIMESTEP )
			{
				accum = 0.f;
			}
		}

		float alpha = fmod(accum, GAME_TIMESTEP) / GAME_TIMESTEP;

		render(alpha, timer);

		StateManager::get().draw(renderer);

		window.display();

	}

	return 0;
}

void RWGame::tick(float dt)
{
	// Clear out any per-tick state.
	engine->clearTickData();
	
	// Process the Engine's background work.
	engine->_work->update();

	static float clockAccumulator = 0.f;
	if (inFocus) {
		engine->gameTime += dt;

		clockAccumulator += dt;
		while( clockAccumulator >= 1.f ) {
			engine->state.minute ++;
			while( engine->state.minute >= 60 ) {
				engine->state.minute = 0;
				engine->state.hour ++;
				while( engine->state.hour >= 24 ) {
					engine->state.hour = 0;
				}
			}
			clockAccumulator -= 1.f;
		}
		
		// Clean up old VisualFX
		for( int i = 0; i < engine->effects.size(); ++i )
		{
			VisualFX* effect = engine->effects[i];
			if( effect->getType() == VisualFX::Particle )
			{
				auto& part = effect->particle;
				if( part.lifetime < 0.f ) continue;
				if( engine->gameTime >= part.starttime + part.lifetime )
				{
					engine->destroyEffect( effect );
					--i;
				}
			}
		}

		for( GameObject* object : engine->objects ) {
			object->_updateLastTransform();
			object->tick(dt);
		}
		
		engine->destroyQueuedObjects();
		engine->state.texts.clear();

		engine->dynamicsWorld->stepSimulation(dt, 2, dt);
		
		if( engine->script ) {
			try {
				engine->script->execute(dt);
			}
			catch( SCMException& ex ) {
				std::cerr << ex.what() << std::endl;
				engine->logger.error( "Script", ex.what() );
				throw;
			}
		}
		
		if ( engine->state.player )
		{
			// Use the current camera position to spawn pedestrians.
			auto p = nextCam.position;
			engine->cleanupTraffic(p);
			engine->createTraffic(p);
		}
	}

	// render() needs two cameras to smoothly interpolate between ticks.
	lastCam = nextCam;
	nextCam	= StateManager::get().states.back()->getCamera();
}

void RWGame::render(float alpha, float time)
{
	lastDraws = getRenderer()->getRenderer()->getDrawCount();
	
	getRenderer()->getRenderer()->swap();
	
	auto size = getWindow().getSize();
	renderer->setViewport(size.x, size.y);
	
	ViewCamera viewCam;
	viewCam.frustum.fov = glm::radians(90);
	if( engine->state.currentCutscene != nullptr && engine->state.cutsceneStartTime >= 0.f )
	{
		auto cutscene = engine->state.currentCutscene;
		float cutsceneTime = std::min(engine->gameTime - engine->state.cutsceneStartTime,
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
	else if( engine->state.cameraFixed )
	{
		viewCam.position = engine->state.cameraPosition;
		viewCam.rotation = engine->state.cameraRotation;
	}
	else
	{
		// There's no cutscene playing - use the camera returned by the State.
		viewCam.position = glm::mix(lastCam.position, nextCam.position, alpha);
		viewCam.rotation = glm::slerp(lastCam.rotation, nextCam.rotation, alpha);
	}

	viewCam.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	if ( engine->state.isCinematic )
	{
		viewCam.frustum.fov *= viewCam.frustum.aspectRatio;
	}

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	renderer->renderWorld(viewCam, alpha);
	
#if 0
	debug->setShaderProgram(engine->renderer.worldProg);
	if( engine->state.player )
	{
		if( engine->state.player->getCharacter()->getCurrentVehicle() )
		{
			auto v = engine->state.player->getCharacter()->getCurrentVehicle();
			for( auto& p : v->dynamicParts )
			{
				if( p.second.body )
				{
					engine->dynamicsWorld->debugDrawObject(p.second.body->getWorldTransform(), p.second.body->getCollisionShape(), btVector3(1.f, 0.f, 0.f));
					engine->dynamicsWorld->debugDrawConstraint(p.second.constraint);
				}
			}
		}
	}
	debug->flush(&engine->renderer);
#endif

	if ( showDebugStats )
	{
		renderDebugStats(time);
	}
	
	drawOnScreenText(engine, renderer);
}

void RWGame::renderDebugStats(float time)
{
	std::stringstream ss;
	ss << "Frametime: " << time << " (FPS " << (1.f/time) << ")\n";
	ss << "Draws: " << lastDraws << " (" << renderer->culled << " Culls)\n";
	
	// Count the number of interesting objects.
	int peds = 0, cars = 0;
	for( GameObject* object : engine->objects )
	{
		switch ( object->type() )
		{
			case GameObject::Character: peds++; break;
			case GameObject::Vehicle: cars++; break;
			default: break;
		}
	}
	
	ss << "P " << peds << " V " << cars << "\n";
	
	if( engine->state.player ) {
		ss << "Player Activity: ";
		if( engine->state.player->getCurrentActivity() ) {
			ss << engine->state.player->getCurrentActivity()->name();
		}
		else {
			ss << "Idle";
		}
		ss << std::endl;
	}
	
	TextRenderer::TextInfo ti;
	ti.text = ss.str();
	ti.font = 2;
	ti.screenPosition = glm::vec2( 10.f, 10.f );
	ti.size = 15.f;
	renderer->text.renderText(ti);

	/*while( engine->log.size() > 0 && engine->log.front().time + 10.f < engine->gameTime ) {
		engine->log.pop_front();
	}

	ti.screenPosition = glm::vec2( 10.f, 500.f );
	ti.size = 15.f;
	for(auto it = engine->log.begin(); it != engine->log.end(); ++it) {
		ti.text = it->message;
		switch(it->type) {
		case GameWorld::LogEntry::Error:
			ti.baseColour = glm::vec3(1.f, 0.f, 0.f);
			break;
		case GameWorld::LogEntry::Warning:
			ti.baseColour = glm::vec3(1.f, 1.f, 0.f);
			break;
		default:
			ti.baseColour = glm::vec3(1.f, 1.f, 1.f);
			break;
		}

		// Interpolate the color
		// c.a = (engine->gameTime - it->time > 5.f) ? 255 - (((engine->gameTime - it->time) - 5.f)/5.f) * 255 : 255;
		// text.setColor(c);

		engine->renderer.text.renderText(ti);
		ti.screenPosition.y -= ti.size;
	}*/
	
	for( int i = 0; i < engine->state.text.size(); )
	{
		if( engine->gameTime > engine->state.text[i].osTextStart + engine->state.text[i].osTextTime )
		{
			engine->state.text.erase(engine->state.text.begin() + i);
		}
		else
		{
			i++;
		}
	}
	
}

void RWGame::globalKeyEvent(const sf::Event& event)
{
	switch (event.key.code) {
	case sf::Keyboard::LBracket:
		engine->state.minute -= 30.f;
		break;
	case sf::Keyboard::RBracket:
		engine->state.minute += 30.f;
		break;
	case sf::Keyboard::Num9:
		timescale *= 0.5f;
		break;
	case sf::Keyboard::Num0:
		timescale *= 2.0f;
		break;
	case sf::Keyboard::F1:
		showDebugStats = ! showDebugStats;
		break;
	default: break;
	}
}
