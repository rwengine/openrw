#ifndef _RWGAME_HPP_
#define _RWGAME_HPP_

#include <core/Logger.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>
#include <script/ScriptMachine.hpp>
#include "game.hpp"

#include "GameConfig.hpp"

#include <SFML/Graphics.hpp>

class PlayerController;
class HttpServer;

class RWGame
{
	Logger log;
	GameConfig config;
	GameState* state;
	GameData* data;
	GameWorld* world;
	// must be allocated after Logger setup.
	GameRenderer* renderer;
    ScriptMachine* script;
	// Background worker
	WorkContext work;
	bool debugScript;
    HttpServer* httpserver = nullptr;
    std::thread* httpserver_thread = nullptr;
	sf::RenderWindow window;
	sf::Clock clock;
	bool inFocus;
	ViewCamera lastCam, nextCam;
	bool showDebugStats;
	bool showDebugPaths;
	bool showDebugPhysics;
	int lastDraws; /// Number of draws issued for the last frame.

	float accum;
	float timescale;
public:

	RWGame(int argc, char* argv[]);
	~RWGame();

	int run();

	/**
	 * Initalizes a new game
	 */
	void newGame();

	GameState* getState() const
	{
		return state;
	}

	GameWorld* getWorld() const
	{
		return world;
	}

	GameData* getGameData() const
	{
		return data;
	}

	GameRenderer* getRenderer() const
	{
		return renderer;
	}

	sf::RenderWindow& getWindow()
	{
		return window;
	}

	ScriptMachine* getScript() const
	{
		return script;
	}

	bool hitWorldRay(glm::vec3 &hit, glm::vec3 &normal, GameObject** object = nullptr)
	{
		auto vc = nextCam;
		glm::vec3 from(vc.position.x, vc.position.y, vc.position.z);
		glm::vec3 tmp = vc.rotation * glm::vec3(1000.f, 0.f, 0.f);

		return hitWorldRay(from, tmp, hit, normal, object);
	}

	bool hitWorldRay(const glm::vec3 &start, const glm::vec3 &direction, glm::vec3 &hit, glm::vec3 &normal, GameObject **object = nullptr)
	{
		auto from = btVector3(start.x, start.y, start.z);
		auto to = btVector3(start.x+direction.x, start.y+direction.y, start.z+direction.z);
		btCollisionWorld::ClosestRayResultCallback ray(from, to);

		world->dynamicsWorld->rayTest(from, to, ray);
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
	
	void startScript(const std::string& name);

	bool hasFocus() const { return inFocus; }

	void saveGame(const std::string& savename);
	void loadGame(const std::string& savename);

	/** shortcut for getWorld()->state.player->getCharacter() */
	PlayerController* getPlayer();

private:
	void tick(float dt);
	void render(float alpha, float dt);
	
	void renderDebugStats(float time, Renderer::ProfileInfo& worldRenderTime);
	void renderDebugPaths(float time);
	void renderProfile();

	void globalKeyEvent(const sf::Event& event);
};

#endif
