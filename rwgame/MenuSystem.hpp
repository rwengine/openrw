#ifndef _GAME_MENUSYSTEM_HPP_
#define _GAME_MENUSYSTEM_HPP_
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <render/GameRenderer.hpp>
#include <functional>
#include <algorithm>

/**
 * Default values for menus that should match the look and feel of the original
 */
namespace MenuDefaults
{
constexpr int kFont = 1;

constexpr const char* kStartGameId = "FET_SAN";
constexpr const char* kResumeGameId = "FEM_RES";
constexpr const char* kLoadGameId = "FET_LG";
constexpr const char* kDebugId = "FEM_DBG";
constexpr const char* kOptionsId = "FET_OPT";
constexpr const char* kQuitGameId = "FET_QG";
}

class Menu
{
	int font;
public:
	
	Menu(int font = MenuDefaults::kFont)
	 : font(font), activeEntry(-1) {}
	
	struct MenuEntry
	{
		GameString text;
		float _size;
		
		MenuEntry(const GameString& n, float size = 30.f)
		    : text(n)
		    , _size(size)
		{
		}
		
		float getHeight() { return _size; }
		
		virtual void draw(int font, bool active, GameRenderer* r, glm::vec2& basis)
		{
			TextRenderer::TextInfo ti;
			ti.font = font;
			ti.screenPosition = basis;
			ti.text = text;
			ti.size = getHeight();
			if( ! active )
			{
				ti.baseColour = glm::u8vec3(255);
			}
			else
			{
				ti.baseColour = glm::u8vec3(255, 255, 0);
			}
			r->text.renderText(ti);
			basis.y += getHeight();
		}
		
		virtual void activate(float clickX, float clickY) = 0;
	};

	struct Entry : public MenuEntry
	{
		std::function<void(void)> callback;

		Entry(const GameString& title,
		      std::function<void(void)> cb,
		      float size)
		    : MenuEntry(title, size)
		    , callback(cb)
		{
		}

		void activate(float clickX, float clickY)
		{
			RW_UNUSED(clickX);
			RW_UNUSED(clickY);
			callback();
		}
	};

	static std::shared_ptr<MenuEntry> lambda(const GameString& n, std::function<void (void)>  callback, float size = 30.f)
	{
		return std::shared_ptr<MenuEntry>(new Entry(n, callback, size));
	}

	static std::shared_ptr<MenuEntry> lambda(const std::string& n, std::function<void (void)>  callback, float size = 30.f)
	{
		return lambda(GameStringUtil::fromString(n), callback, size);
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
	
	void draw(GameRenderer* r)
	{
		glm::vec2 basis(offset);
		for(size_t i = 0;
			i < entries.size();
			++i)
		{
			bool active = false;
			if(activeEntry >= 0 && i == (unsigned) activeEntry)
			{
				active = true;
			}
			entries[i]->draw(font, active, r, basis);
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
		if(activeEntry >= 0 && (unsigned) activeEntry < entries.size()) {
			entries[activeEntry]->activate(0.f, 0.f);
		}
	}
	
	void move(int movement)
	{
		activeEntry += movement;
		if (activeEntry >= int(entries.size())) {
			activeEntry = 0;
		}
		else if (activeEntry < 0) {
			activeEntry = entries.size() - 1;
		}
	}
};

#endif
