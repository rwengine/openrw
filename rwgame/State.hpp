#ifndef _GAME_STATE_HPP_
#define _GAME_STATE_HPP_
#include <functional>
#include <queue>
#include <SFML/Graphics/RenderWindow.hpp>
#include <render/ViewCamera.hpp>
#include "MenuSystem.hpp"
#include <glm/gtc/quaternion.hpp>

class RWGame;
class GameWorld;

struct State
{
	// Helper for global menu behaviour
	Menu* currentMenu;
	Menu* nextMenu;

	RWGame* game;
	
	State(RWGame* game)
	 : currentMenu(nullptr), nextMenu(nullptr), game(game) {}
	
	virtual void enter() = 0;
	virtual void exit() = 0;
	
	virtual void tick(float dt) = 0;
	
	virtual void draw(sf::RenderWindow& w)
	{
		if(getCurrentMenu()) {
			getCurrentMenu()->draw(w);
		}
	}
	
	virtual ~State() {
		if(getCurrentMenu()) {
			delete getCurrentMenu();
		}
	}
	
	void enterMenu(Menu* menu)
	{
		nextMenu = menu;
	}
	
	Menu* getCurrentMenu()
	{
		if(nextMenu) {
			if(currentMenu) {
				delete currentMenu;
			}
			currentMenu = nextMenu;
			nextMenu = nullptr;
		}
		return currentMenu;
	}
	
	virtual void handleEvent(const sf::Event& e)
	{
		auto m = getCurrentMenu();
		if(! m) return;
		switch(e.type) {
			case sf::Event::MouseButtonReleased:
				m->click(e.mouseButton.x, e.mouseButton.y);
				break;
			case sf::Event::MouseMoved:
				m->hover(e.mouseMove.x, e.mouseMove.y);
				break;
			case sf::Event::KeyPressed:
				switch(e.key.code) {
				default: break;
					case sf::Keyboard::Up:
						m->move(-1);
						break;
					case sf::Keyboard::Down:
						m->move(1);
						break;
					case sf::Keyboard::Return:
						m->activate();
						break;
				}
			default: break;
		};
	}

	virtual const ViewCamera& getCamera();

	GameWorld* getWorld();
	sf::RenderWindow& getWindow();
};

struct StateManager
{
	static StateManager& get()
	{
		static StateManager m;
		return m;
	}
	
	std::deque<State*> states;
	
	void enter(State* state)
	{
		states.push_back(state);
		state->enter();
	}

	void exec(State* state)
	{
		exit();
		enter(state);
	}
	
	void tick(float dt)
	{
		states.back()->tick(dt);
	}
	
	void draw(sf::RenderWindow& w)
	{
		states.back()->draw(w);
	}
	
	void exit()
	{
		// TODO: Resole states being leaked.
		states.back()->exit();
		states.pop_back();
		if(states.size() > 0) {
			states.back()->enter();
		}
	}
};

#endif
