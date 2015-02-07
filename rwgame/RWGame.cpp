#include "RWGame.hpp"
#include "State.hpp"
#include "loadingstate.hpp"

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

RWGame::RWGame(const std::string& gamepath, int argc, char* argv[])
	: engine(nullptr), inFocus(true),
	  accum(0.f), timescale(1.f)
{
	size_t w = GAME_WINDOW_WIDTH, h = GAME_WINDOW_HEIGHT;
	bool fullscreen = false;

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

	// Initalize all the archives.
	engine->gameData.loadIMG("/models/gta3");
	engine->gameData.loadIMG("/models/txd");
	engine->gameData.loadIMG("/anim/cuts");
	
	// Set up text renderer
	engine->renderer.text.setFontTexture(0, "pager");
	engine->renderer.text.setFontTexture(1, "font1");
	engine->renderer.text.setFontTexture(2, "font2");

	/// @TODO expand this here.
	engine->load();
	debug = new DebugDraw;
	debug->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawConstraintLimits);
	engine->dynamicsWorld->setDebugDrawer(debug);

	engine->gameData.loadDynamicObjects(gamepath + "/data/object.dat");

	/// @TODO language choices.
	engine->gameData.loadGXT("english.gxt");
	
	for(int m = 0; m < MAP_BLOCK_SIZE; ++m)
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		engine->gameData.loadTXD(name + ".txd");
	}

	StateManager::get().enter(new LoadingState(this));

	engine->logInfo("Started");
}

RWGame::~RWGame()
{
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
			default: break;
			}

			StateManager::get().states.back()->handleEvent(event);
		}
		
		if(! window.isOpen() )
		{
			break;
		}

		accum += clock.restart().asSeconds() * timescale;

		while ( accum >= GAME_TIMESTEP ) {

			if( ! getWorld()->isPaused() )
			{
				StateManager::get().tick(GAME_TIMESTEP);

				tick(GAME_TIMESTEP);
			}
			
			accum -= GAME_TIMESTEP;
		}

		float alpha = accum / GAME_TIMESTEP;

		render(alpha);

		StateManager::get().draw(&engine->renderer);

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
			if( engine->state.minute >= 60 ) {
				engine->state.minute = 0;
				engine->state.hour ++;
				if( engine->state.hour >= 24 ) {
					engine->state.hour = 0;
				}
			}
			clockAccumulator -= 1.f;
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
				engine->logError( ex.what() );
				throw;
			}
		}
	}

	// render() needs two cameras to smoothly interpolate between ticks.
	lastCam = nextCam;
	nextCam	= StateManager::get().states.back()->getCamera();
}

void RWGame::render(float alpha)
{
	auto size = getWindow().getSize();
	engine->renderer.getRenderer()->setViewport({size.x, size.y});
	
	ViewCamera viewCam;
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
		float cinematicAspect = 19.f/10.f;
		viewCam.frustum.fov *= viewCam.frustum.aspectRatio;
	}

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	engine->renderer.renderWorld(viewCam, alpha);
	
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
	
	std::stringstream ss;
	ss << std::setfill('0') << "Time: " << std::setw(2) << engine->getHour()
		<< ":" << std::setw(2) << engine->getMinute() << " (" << engine->gameTime << "s)\n";
	ss << "View: " << viewCam.position.x << " " << viewCam.position.y << " " << viewCam.position.z << "\n";
	ss << "Drawn " << engine->renderer.rendered << " / " << engine->renderer.culled << " Culled " << " " << engine->renderer.frames << "	" << engine->renderer.geoms << "\n";
	if( engine->state.player ) {
		ss << "Activity: ";
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
	ti.size = 20.f;
	engine->renderer.text.renderText(ti);

	while( engine->log.size() > 0 && engine->log.front().time + 10.f < engine->gameTime ) {
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
	}
	
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

	for(OnscreenText& t : engine->state.text)
	{
		float fontSize = 20.f;
		switch(t.osTextStyle)
		{
			default:
				fontSize = 20.f;
				break;
			case 1:
				fontSize = 40.f;
				break;
			case 2:
				fontSize = 20.f;
				break;
		}
		
		ti.size = fontSize;
		ti.screenPosition = glm::vec2(0.f);
		ti.font = 0;
		if(t.osTextStyle == 1)
		{
			ti.screenPosition = glm::vec2(500.f, 500.f);
		}
		else if(t.osTextStyle == 2)
		{
			ti.screenPosition = glm::vec2(500.f, 500.f);
		}
		else if(t.osTextStyle == 12)
		{
			ti.screenPosition = glm::vec2(20.f, 20.f);
			ti.font = 2;
// 			messageText.setPosition(15.f, 15.f);
			
// 			// Insert line breaks into the message string.
// 			auto m = messageText.getString();
// 			const float boxWidth = 250.f;
// 			int lastSpace = 0;
// 			float lineLength = 0.f, wordLength = 0.f;
// 			for( int c = 0; c < m.getSize(); ++c )
// 			{
// 				if(m[c] == ' ')
// 				{
// 					lastSpace = c;
// 					lineLength += wordLength;
// 					wordLength = 0.f;
// 				}
// 				
// 				auto& metrics = font.getGlyph(m[c], fontSize, false);
// 				wordLength += metrics.bounds.width;
// 				
// 				if( lineLength + wordLength > boxWidth )
// 				{
// 					m[lastSpace] = '\n';
// 					lineLength = 0.f;
// 				}
// 			}
// 			messageText.setString(m);
// 			
// 			auto bds = messageText.getGlobalBounds();
// 			sf::RectangleShape bg(sf::Vector2f(bds.width, bds.height) + sf::Vector2f(10.f, 10.f));
// 			bg.setFillColor(sf::Color::Black);
// 			bg.setPosition(sf::Vector2f(bds.left, bds.top) - sf::Vector2f(5.f, 5.f));
// 			window.draw(bg);
		}
		else
		{
			float lowerBar = 550.f;
			ti.screenPosition = glm::vec2(300.f, lowerBar);
		}
		
		ti.text = t.osTextString;
		engine->renderer.text.renderText(ti);
	}

	/*for(auto& t : engine->state.texts) {
		sf::Text messageText(t.text, font, 15);

		glm::vec2 scpos(t.position.x, t.position.y);
		auto s = window.getSize();
		scpos /= glm::vec2(640.f, 480.f);
		scpos *= glm::vec2(s.x, s.y);

		messageText.setPosition(scpos.x, scpos.y);

		window.draw(messageText);
	}*/
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
	}
}
