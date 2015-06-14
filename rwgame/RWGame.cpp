#include "RWGame.hpp"
#include "State.hpp"
#include "loadingstate.hpp"
#include "DrawUI.hpp"
#include "ingamestate.hpp"
#include "menustate.hpp"

#include <objects/GameObject.hpp>
#include <engine/GameState.hpp>
#include <engine/SaveGame.hpp>
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>
#include <render/DebugDraw.hpp>

#include <script/ScriptMachine.hpp>
#include <script/modules/VMModule.hpp>
#include <script/modules/GameModule.hpp>
#include <script/modules/ObjectModule.hpp>

#include <data/CutsceneData.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

DebugDraw* debug;

StdOutReciever logPrinter;

RWGame::RWGame(const std::string& gamepath, int argc, char* argv[])
	: state(nullptr), world(nullptr), renderer(nullptr), script(nullptr), inFocus(true),
	showDebugStats(false), showDebugPaths(false), showDebugPhysics(false),
	accum(0.f), timescale(1.f)
{
	size_t w = GAME_WINDOW_WIDTH, h = GAME_WINDOW_HEIGHT;
	bool fullscreen = false;
	bool newgame = false;
	bool test = false;
	bool debugscript = false;

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
		if( strcmp( "--test", argv[i] ) == 0 )
		{
			test = true;
		}
		if( strcmp( "--debug", argv[i] ) == 0 )
		{
			debugscript = true;
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
	
	log.addReciever(&logPrinter);	
	log.info("Game", "Game directory: " + gamepath);
	
	if(! GameData::isValidGameDirectory(gamepath) )
	{
		std::string envname(ENV_GAME_PATH_NAME);
		throw std::runtime_error("Invalid game directory path, is " +envname+ " set?");
	}

	data = new GameData(&log, &work, gamepath);

	// Initalize all the archives.
	data->loadIMG("/models/gta3");
	//engine->data.loadIMG("/models/txd");
	data->loadIMG("/anim/cuts");
	
	data->load();
	
	// Initialize renderer
	renderer = new GameRenderer(&log, data);
	
	// Set up text renderer
	renderer->text.setFontTexture(0, "pager");
	renderer->text.setFontTexture(1, "font1");
	renderer->text.setFontTexture(2, "font2");

	debug = new DebugDraw;
	debug->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawConstraintLimits);
	debug->setShaderProgram(renderer->worldProg);

	data->loadDynamicObjects(gamepath + "/data/object.dat");

	/// @TODO language choices.
	data->loadGXT("english.gxt");
	
	getRenderer()->water.setWaterTable(data->waterHeights, 48, data->realWater, 128*128);
	
	for(int m = 0; m < MAP_BLOCK_SIZE; ++m)
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		data->loadTXD(name + ".txd");
	}

	auto loading = new LoadingState(this);
	if( newgame )
	{
		loading->setNextState(new IngameState(this,true,test));
	}
	else
	{
		loading->setNextState(new MenuState(this));
	}
	
	StateManager::get().enter(loading);

	log.info("Game", "Started");
}

RWGame::~RWGame()
{
	delete script;
	delete renderer;
	delete world;
	delete state;
}

void RWGame::newGame()
{
	if( state != nullptr )
	{
		log.error("Game", "Cannot start a new game: game is already running.");
		return;
	}

	state = new GameState;
	world = new GameWorld(&log, &work, data);
	world->dynamicsWorld->setDebugDrawer(debug);

	// Associate the new world with the new state and vice versa
	state->world = world;
	world->state = state;

	for(std::map<std::string, std::string>::iterator it = world->data->iplLocations.begin();
		it != world->data->iplLocations.end();
		++it) {
		world->data->loadZone(it->second);
		world->placeItems(it->second);
	}
	
}

void RWGame::saveGame(const std::string& savename)
{
	// Save games without a script don't make much sense at the moment
	if( script )
	{
		SaveGame::writeScript(*script, savename+".script");
		SaveGame::writeState(*state, savename+".state");
		SaveGame::writeObjects(*world, savename+".world");
	}
}

void RWGame::loadGame(const std::string& savename)
{
	delete state->world;
	delete state->script;
	state = nullptr;

	newGame();

	startScript("data/main.scm");

	if(! SaveGame::loadGame(*state, "GTA3sf1.b") )
	{
		log.error("Game", "Failed to load game");
	}
}

void RWGame::startScript(const std::string& name)
{
	SCMFile* f = world->data->loadSCM(name);
	if( f ) {
		if( script )
		{
			delete script;
		}
		
		SCMOpcodes* opcodes = new SCMOpcodes;
		opcodes->modules.push_back(new VMModule);
		opcodes->modules.push_back(new GameModule);
		opcodes->modules.push_back(new ObjectModule);

		script = new ScriptMachine(state, f, opcodes);
		
		// Set up breakpoint handler
		script->setBreakpointHandler(
			[&](const SCMBreakpoint& bp)
			{
				log.info("Script", "Breakpoint hit!");
				std::stringstream ss;
				ss << " " << bp.function->description << ".";
				ss << " Args:";
				for(int a = 0; a < bp.args->getParameters().size(); a++)
				{
					auto& arg = bp.args->getParameters()[a];
					ss << " " << arg.integerValue();
					if( a != bp.args->getParameters().size()-1 )
					{
						ss << ",";
					}
				}
				
				log.info("Script", ss.str());
			});
		script->addBreakpoint(0);
		state->script = script;
	}
	else {
		log.error("Game", "Failed to load SCM: " + name);
	}
}

PlayerController *RWGame::getPlayer()
{
	auto object = world->findObject(state->playerObject);
	if( object )
	{
		auto controller = static_cast<CharacterObject*>(object)->controller;
		return static_cast<PlayerController*>(controller);
	}
	return nullptr;
}

int RWGame::run()
{
	clock.restart();
	
	// Loop until the window is closed or we run out of state.
	while (window.isOpen() && StateManager::get().states.size()) {
		State* state = StateManager::get().states.back();
		
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

			state->handleEvent(event);
		}
		
		if(! window.isOpen() )
		{
			break;
		}

		float timer = clock.restart().asSeconds();
		accum += timer * timescale;

		while ( accum >= GAME_TIMESTEP ) {
			StateManager::get().tick(GAME_TIMESTEP);

			tick(GAME_TIMESTEP);
			
			accum -= GAME_TIMESTEP;
			
			// Throw away time if the accumulator reaches too high.
			if ( accum > GAME_TIMESTEP * 5.f )
			{
				accum = 0.f;
			}
		}

		float alpha = fmod(accum, GAME_TIMESTEP) / GAME_TIMESTEP;
		if( ! state->shouldWorldUpdate() )
		{
			alpha = 1.f;
		}

		render(alpha, timer);

		StateManager::get().draw(renderer);

		window.display();
	}

	return 0;
}

void RWGame::tick(float dt)
{
	// Process the Engine's background work.
	world->_work->update();
	
	State* currState = StateManager::get().states.back();
	
	static float clockAccumulator = 0.f;
	if ( currState->shouldWorldUpdate() ) {
		// Clear out any per-tick state.
		world->clearTickData();

		state->gameTime += dt;

		clockAccumulator += dt;
		while( clockAccumulator >= 1.f ) {
			world->state->minute ++;
			while( state->minute >= 60 ) {
				state->minute = 0;
				state->hour ++;
				while( state->hour >= 24 ) {
					state->hour = 0;
				}
			}
			clockAccumulator -= 1.f;
		}
		
		// Clean up old VisualFX
		for( int i = 0; i < world->effects.size(); ++i )
		{
			VisualFX* effect = world->effects[i];
			if( effect->getType() == VisualFX::Particle )
			{
				auto& part = effect->particle;
				if( part.lifetime < 0.f ) continue;
				if( world->getGameTime() >= part.starttime + part.lifetime )
				{
					world->destroyEffect( effect );
					--i;
				}
			}
		}

		for( auto& p : world->objects ) {
			GameObject* object = p.second;
			object->_updateLastTransform();
			object->tick(dt);
		}
		
		world->destroyQueuedObjects();
		state->texts.clear();

		for( int i = 0; i < state->text.size(); )
		{
			auto& text = state->text[i];
			if( world->getGameTime() > text.osTextStart + text.osTextTime )
			{
				state->text.erase(state->text.begin() + i);
			}
			else
			{
				i++;
			}
		}

		world->dynamicsWorld->stepSimulation(dt, 2, dt);
		
		if( script ) {
			try {
				script->execute(dt);
			}
			catch( SCMException& ex ) {
				std::cerr << ex.what() << std::endl;
				log.error( "Script", ex.what() );
				throw;
			}
		}
		
		if ( state->playerObject )
		{
			// Use the current camera position to spawn pedestrians.
			auto p = nextCam.position;
			world->cleanupTraffic(p);
			world->createTraffic(p);
		}
	}
	
	// render() needs two cameras to smoothly interpolate between ticks.
	lastCam = nextCam;
	nextCam = currState->getCamera();
}

void RWGame::render(float alpha, float time)
{
	lastDraws = getRenderer()->getRenderer()->getDrawCount();
	
	getRenderer()->getRenderer()->swap();
	
	auto size = getWindow().getSize();
	renderer->setViewport(size.x, size.y);
	
	ViewCamera viewCam;
	viewCam.frustum.fov = glm::radians(90.f);
	if( state->currentCutscene != nullptr && state->cutsceneStartTime >= 0.f )
	{
		auto cutscene = state->currentCutscene;
		float cutsceneTime = std::min(world->getGameTime() - state->cutsceneStartTime,
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
	else if( state->cameraFixed )
	{
		viewCam.position = state->cameraPosition;
		viewCam.rotation = state->cameraRotation;
	}
	else
	{
		// There's no cutscene playing - use the camera returned by the State.
		viewCam.position = glm::mix(lastCam.position, nextCam.position, alpha);
		viewCam.rotation = glm::slerp(lastCam.rotation, nextCam.rotation, alpha);
	}

	viewCam.frustum.aspectRatio = window.getSize().x / (float) window.getSize().y;
	
	if ( state->isCinematic )
	{
		viewCam.frustum.fov *= viewCam.frustum.aspectRatio;
	}

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	renderer->getRenderer()->pushDebugGroup("World");

	renderer->renderWorld(world, viewCam, alpha);

	auto rendertime = renderer->getRenderer()->popDebugGroup();

	if( showDebugPaths )
	{
		renderDebugPaths(time);
	}

	if ( showDebugStats )
	{
		renderDebugStats(time, rendertime);
	}

	if( showDebugPhysics )
	{
		if( world )
		{
			world->dynamicsWorld->debugDrawWorld();
			debug->flush(renderer);
		}
	}
	
	drawOnScreenText(world, renderer);
}

void RWGame::renderDebugStats(float time, Renderer::ProfileInfo& worldRenderTime)
{
	// Turn time into milliseconds
	float time_ms = time * 1000.f;
	constexpr size_t average_every_frame = 15;
	static float times[average_every_frame];
	static size_t times_index = 0;
	static float time_average = 0;
	times[times_index++] = time_ms;
	if (times_index >= average_every_frame) {
		times_index = 0;
		time_average = 0;

		for (int i = 0; i < average_every_frame; ++i) {
			time_average += times[i];
		}
		time_average /= average_every_frame;
	}

	std::map<std::string, Renderer::ProfileInfo*> profGroups {
		{"Objects", &renderer->profObjects},
		{"Effects", &renderer->profEffects},
		{"Sky", &renderer->profSky},
		{"Water", &renderer->profWater},
	};

	std::stringstream ss;
	ss << "Frametime: " << time_ms << " (FPS " << (1.f/time) << ")\n";
	ss << "Average (per " << average_every_frame << " frames); Frametime: " << time_average << " (FPS " << (1000.f/time_average) << ")\n";
	ss << "Draws: " << lastDraws << " (" << renderer->culled << " Culls)\n";
	ss << " Texture binds: " << renderer->getRenderer()->getTextureCount() << "\n";
	ss << " Buffer binds: " << renderer->getRenderer()->getBufferCount() << "\n";
	ss << " World time: " << (worldRenderTime.duration/1000000) << "ms\n";
	for(auto& perf : profGroups)
	{
		ss << "  " << perf.first << ": "
		<< perf.second->draws << " draws " << perf.second->primitives << " prims "
		<< (perf.second->duration/1000000) << "ms\n";
	}
	
	// Count the number of interesting objects.
	int peds = 0, cars = 0;
	for( auto& p : world->objects )
	{
		GameObject* object = p.second;
		switch ( object->type() )
		{
			case GameObject::Character: peds++; break;
			case GameObject::Vehicle: cars++; break;
			default: break;
		}
	}
	
	ss << "P " << peds << " V " << cars << "\n";
	
	if( state->playerObject ) {
		ss << "Player (" << state->playerObject << ")\n";
		auto object = world->findObject(state->playerObject);
		auto player = static_cast<CharacterObject*>(object)->controller;
		ss << "Player Activity: ";
		if( player->getCurrentActivity() ) {
			ss << player->getCurrentActivity()->name();
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
}

void RWGame::renderDebugPaths(float time)
{
	btVector3 roadColour(1.f, 0.f, 0.f);
	btVector3 pedColour(0.f, 0.f, 1.f);
	
	for( AIGraphNode* n : world->aigraph.nodes )
	{
		btVector3 p( n->position.x, n->position.y, n->position.z );
		auto& col = n->type == AIGraphNode::Pedestrian ? pedColour : roadColour;
		debug->drawLine( p - btVector3(0.f, 0.f, 1.f), p + btVector3(0.f, 0.f, 1.f), col);
		debug->drawLine( p - btVector3(1.f, 0.f, 0.f), p + btVector3(1.f, 0.f, 0.f), col);
		debug->drawLine( p - btVector3(0.f, 1.f, 0.f), p + btVector3(0.f, 1.f, 0.f), col);

		for( AIGraphNode* c : n->connections )
		{
			btVector3 f( c->position.x, c->position.y, c->position.z );
			debug->drawLine( p, f, col);
		}
	}

	debug->flush(renderer);
}

void RWGame::globalKeyEvent(const sf::Event& event)
{
	switch (event.key.code) {
	case sf::Keyboard::LBracket:
		state->minute -= 30.f;
		break;
	case sf::Keyboard::RBracket:
		state->minute += 30.f;
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
	case sf::Keyboard::F2:
		showDebugPaths = ! showDebugPaths;
		break;
	case sf::Keyboard::F3:
		showDebugPhysics = ! showDebugPhysics;
		break;
	default: break;
	}
}
