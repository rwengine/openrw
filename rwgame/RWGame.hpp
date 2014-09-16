#ifndef _RWGAME_HPP_
#define _RWGAME_HPP_
#include <engine/GameWorld.hpp>
#include "game.hpp"

#include <SFML/Graphics.hpp>

class RWGame
{
	GameWorld* engine;
	sf::RenderWindow window;
	sf::Clock clock;
	bool inFocus;
	ViewCamera lastCam, nextCam;

	float accum;
	float timescale;

	sf::Font font;
public:

	RWGame(const std::string& gamepath);
	~RWGame();

	int run();

	GameWorld* getWorld() const
	{
		return engine;
	}

	sf::RenderWindow& getWindow()
	{
		return window;
	}

	sf::Font& getFont()
	{
		return font;
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

		engine->dynamicsWorld->rayTest(from, to, ray);
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

private:
	void tick(float dt);
	void render(float alpha);

	void globalKeyEvent(const sf::Event& event);
};

#endif
