#ifndef _GAME_STATE_HPP_
#define _GAME_STATE_HPP_
#include <functional>
#include <queue>
#include <SFML/Graphics/RenderWindow.hpp>
#include "MenuSystem.hpp"

struct State
{
	// Helper for global menu behaviour
	Menu* currentMenu;
	
	State()
	 : currentMenu(nullptr) {}
	
	virtual void enter() = 0;
	virtual void exit() = 0;
	
	virtual void tick(float dt) = 0;
	
	virtual void draw(sf::RenderWindow& w)
	{
		if(currentMenu) {
			currentMenu->draw(w);
		}
	}
	
	void enterMenu(Menu* menu)
	{
		currentMenu = menu;
	}
	
	virtual void handleEvent(const sf::Event& e)
	{
		switch(e.type) {
			case sf::Event::MouseButtonReleased:
				if(currentMenu) {
					currentMenu->click(e.mouseButton.x, e.mouseButton.y);
				}
				break;
			case sf::Event::MouseMoved:
				if(currentMenu) {
					currentMenu->hover(e.mouseMove.x, e.mouseMove.y);
				}
				break;
			case sf::Event::KeyPressed:
				if(currentMenu) {
					switch(e.key.code) {
						case sf::Keyboard::Up:
							currentMenu->move(-1);
							break;
						case sf::Keyboard::Down:
							currentMenu->move(1);
							break;
						case sf::Keyboard::Return:
							currentMenu->activate();
							break;
					}
				}
			default: break;
		};
	}
};

struct GenericState : public State
{
	typedef std::function<void (State*)> StateChange;
	typedef std::function<void (State*, float)> Tick;
	typedef std::function<void (State*, const sf::Event&)> Event;
	
	StateChange enter_lambda;
	Tick tick_lambda;
	StateChange exit_lambda;
	Event event_lambda;
	
	GenericState(StateChange start, Tick think, StateChange end, Event event)
	 : enter_lambda(start), tick_lambda(think), 
	   exit_lambda(end), event_lambda(event) {}
	 
	virtual void enter() { enter_lambda(this); }
	virtual void exit() { exit_lambda(this); }
	
	virtual void tick(float dt) { tick_lambda(this, dt); } 
	
	virtual void handleEvent(const sf::Event& event) { 
		event_lambda(this, event); 
		State::handleEvent(event);
	}
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