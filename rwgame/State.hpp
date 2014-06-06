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
	Menu* nextMenu;
	
	State()
	 : currentMenu(nullptr), nextMenu(nullptr) {}
	
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
