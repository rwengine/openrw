#ifndef _GAME_MENUSYSTEM_HPP_
#define _GAME_MENUSYSTEM_HPP_
#include <string>
#include <memory>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <functional>

class Menu
{
	sf::Font font;
public:
	
	Menu(const sf::Font& font)
	 : font(font), activeEntry(-1) {}
	
	struct MenuEntry
	{
		std::string name;
		float _size;
		
		MenuEntry(const std::string& n, float size = 38.f) : name(n), _size(size) {}
		
		float getHeight() { return _size; }
		
		virtual void draw(const sf::Font& font, sf::RenderWindow& window, glm::vec2& basis)
		{
			sf::Text t;
			t.setFont(font);
			t.setPosition(basis.x + 6, basis.y + 2);
			t.setString(name);
			auto cSize = getHeight() - 10.f;
			t.setCharacterSize(cSize);
			window.draw(t);
			basis.y += getHeight();
		}
		
		virtual void activate(float clickX, float clickY) = 0;
	};
	
	struct Entry : public MenuEntry
	{
		std::function<void (void)> callback;
		
		Entry(const std::string& title, std::function<void (void)> cb, float size)
		 : MenuEntry(title, size), callback(cb) {}
		 
		 void activate(float clickX, float clickY) { callback(); }
	};
	
	static std::shared_ptr<MenuEntry> lambda(const std::string& n, std::function<void (void)>  callback, float size = 38.f)
	{
		return std::shared_ptr<MenuEntry>(new Entry(n, callback, size));
	}
	
	std::vector<std::shared_ptr<MenuEntry>> entries;
	
	/**
	 * Active Entry index
	 */
	int activeEntry;
	
	glm::vec2 offset;
	
	void addEntry(std::shared_ptr<MenuEntry> entry)
	{
		entries.push_back(entry);
	}
	
	void draw(sf::RenderWindow& window)
	{
		glm::vec2 basis(offset);
		for(size_t i = 0;
			i < entries.size();
			++i)
		{
			if(activeEntry >= 0 && i == activeEntry) {
				sf::RectangleShape rs(sf::Vector2f(250.f, entries[i]->getHeight()));
				rs.setPosition(basis.x, basis.y);
				rs.setFillColor(sf::Color::Cyan);
				window.draw(rs);
			}
			entries[i]->draw(font, window, basis);
		}
	}
	
	void hover(const float x, const float y)
	{
		glm::vec2 c(x - offset.x, y - offset.y);
		for(size_t i = 0;
			i < entries.size();
			++i)
		{
			if( c.y > 0.f && c.y < entries[i]->getHeight() ) {
				activeEntry = i;
				return;
			}
			else {
				c.y -= entries[i]->getHeight();
			}
		}
	}
	
	void click(const float x, const float y)
	{
		glm::vec2 c(x - offset.x, y - offset.y);
		for(auto it = entries.begin();
			it != entries.end();
			++it)
		{
			if( c.y > 0.f && c.y < (*it)->getHeight() ) {
				(*it)->activate(c.x, c.y);
				return;
			}
			else {
				c.y -= (*it)->getHeight();
			}
		}
	}
	
	// Activates the menu entry at the current active index.
	void activate()
	{
		if(activeEntry < entries.size()) {
			entries[activeEntry]->activate(0.f, 0.f);
		}
	}
	
	void move(int movement)
	{
		activeEntry = std::min<const int>(entries.size()-1, std::max<const int>(0, activeEntry + movement));
	}
};

#endif
