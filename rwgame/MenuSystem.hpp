#ifndef _GAME_MENUSYSTEM_HPP_
#define _GAME_MENUSYSTEM_HPP_

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include <fonts/GameTexts.hpp>
#include <rw/debug.hpp>

class GameRenderer;

/**
 * Default values for menus that should match the look and feel of the original
 */
namespace MenuDefaults {
constexpr int kFont = 1;
constexpr const char* kStartGameId = "FET_SAN";
constexpr const char* kResumeGameId = "FEM_RES";
constexpr const char* kLoadGameId = "FET_LG";
constexpr const char* kDebugId = "FEM_DBG";
constexpr const char* kOptionsId = "FET_OPT";
constexpr const char* kQuitGameId = "FET_QG";
}

/**
 * @brief Implements user navigable menus
 *
 * This is a temporary implementation
 */
class Menu {
public:
    /**
     * @brief Handles rendering and dispatch of menu items
     */
    class MenuEntry {
        GameString text;
        std::function<void(void)> callback;

    public:
        MenuEntry(const std::string& n, const std::function<void(void)>& cb)
            : text(GameStringUtil::fromString(n, FONT_PRICEDOWN)), callback(cb) {
        }
        MenuEntry(const GameString& n, const std::function<void(void)>& cb)
            : text(n), callback(cb) {
        }

        void draw(font_t font, float size, bool active, GameRenderer& r,
                  glm::vec2& basis);

        void activate(float clickX, float clickY) {
            RW_UNUSED(clickX);
            RW_UNUSED(clickY);
            callback();
        }
    };

    Menu(std::initializer_list<MenuEntry>&& initial,
         const glm::vec2& offset = glm::vec2(), int font = MenuDefaults::kFont,
         float size = 30.f)
        : activeEntry(-1)
        , offset(offset)
        , font(font)
        , size(size)
        , entries(initial) {
    }

    Menu& lambda(const GameString& n, std::function<void()> callback) {
        entries.emplace_back(n, callback);
        return *this;
    }

    Menu& lambda(const std::string& n, std::function<void(void)> callback) {
        entries.emplace_back(GameStringUtil::fromString(n, FONT_PRICEDOWN), callback);
        return *this;
    }

    /**
     * Active Entry index
     */
    int activeEntry;

    void draw(GameRenderer& r);

    void hover(const float x, const float y);

    void click(const float x, const float y);

    // Activates the menu entry at the current active index.
    void activate();

    void move(int movement);

    const std::vector<MenuEntry>& getEntries() const {
        return entries;
    }

private:
    glm::vec2 offset{};
    int font;
    float size;
    std::vector<MenuEntry> entries;
};

#endif
