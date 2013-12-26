#ifndef _GAME_MENUSYSTEM_HPP_
#define _GAME_MENUSYSTEM_HPP_
#include <string>
#include <memory>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <functional>

class Menu
{
	sf::Font font;
public:
	
	Menu(const sf::Font& font)
	 : font(font) {}
	
	struct MenuEntry
	{
		std::string name;
		
		MenuEntry(const std::string& n) : name(n) {}
		
		float getHeight() { return 30.f; }
		
		virtual void draw(const sf::Font& font, sf::RenderWindow& window, sf::Vector2f& basis)
		{
			sf::Text t;
			t.setFont(font);
			t.setPosition(basis);
			t.setString(name);
			window.draw(t);
			basis.y += getHeight();
		}
		
		virtual void activate(sf::Vector2f click) = 0;
	};
	
	struct Entry : public MenuEntry
	{
		std::function<void (void)> callback;
		
		Entry(const std::string& title, std::function<void (void)> cb)
		 : MenuEntry(title), callback(cb) {}
		 
		 void activate(sf::Vector2f click) { callback(); }
	};
	
	static std::shared_ptr<MenuEntry> lambda(const std::string& n, std::function<void (void)>  callback)
	{
		return std::shared_ptr<MenuEntry>(new Entry(n, callback));
	}
	
	std::vector<std::shared_ptr<MenuEntry>> entries;
	
	sf::Vector2f offset;
	
	void addEntry(std::shared_ptr<MenuEntry> entry)
	{
		entries.push_back(entry);
	}
	
	void draw(sf::RenderWindow& window)
	{
		sf::Vector2f basis(offset);
		for(auto it = entries.begin();
			it != entries.end();
			++it)
		{
			(*it)->draw(font, window, basis);
		}
	}
	
	void click(const float x, const float y)
	{
		sf::Vector2f c(x - offset.x, y - offset.y);
		for(auto it = entries.begin();
			it != entries.end();
			++it)
		{
			if( c.y > 0.f && c.y < (*it)->getHeight() ) {
				(*it)->activate(c);
				return;
			}
			else {
				c.y -= (*it)->getHeight();
			}
		}
	}
};

#endif