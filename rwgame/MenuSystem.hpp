#ifndef _GAME_MENUSYSTEM_HPP_
#define _GAME_MENUSYSTEM_HPP_
#include <algorithm>
#include <functional>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <render/GameRenderer.hpp>
#include <rw/defines.hpp>

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
        MenuEntry(const std::string& n, std::function<void(void)> cb)
            : text(GameStringUtil::fromString(n)), callback(cb) {
        }
        MenuEntry(const GameString& n, std::function<void(void)> cb)
            : text(n), callback(cb) {
        }

        void draw(int font, float size, bool active, GameRenderer* r,
                  glm::vec2& basis) {
            TextRenderer::TextInfo ti;
            ti.font = font;
            ti.screenPosition = basis;
            ti.text = text;
            ti.size = size;
            if (!active) {
                ti.baseColour = glm::u8vec3(255);
            } else {
                ti.baseColour = glm::u8vec3(255, 255, 0);
            }
            r->text.renderText(ti);
            basis.y += size;
        }

        void activate(float clickX, float clickY) {
            RW_UNUSED(clickX);
            RW_UNUSED(clickY);
            callback();
        }
    };

    Menu(std::vector<MenuEntry> initial, int font = MenuDefaults::kFont,
         float size = 30.f)
        : activeEntry(-1), font(font), size(size), entries(std::move(initial)) {
    }

    /**
     * @brief creates a menu from the given menu items
     * @return a shared pointer to the menu with the items
     */
    static std::shared_ptr<Menu> create(std::vector<MenuEntry> items,
                                        int font = MenuDefaults::kFont,
                                        float size = 30.f) {
        return std::make_shared<Menu>(std::move(items), font, size);
    }

    Menu& lambda(const GameString& n, std::function<void()> callback) {
        entries.emplace_back(n, callback);
        return *this;
    }

    Menu& lambda(const std::string& n, std::function<void(void)> callback) {
        entries.emplace_back(GameStringUtil::fromString(n), callback);
        return *this;
    }

    /**
     * Active Entry index
     */
    int activeEntry;

    glm::vec2 offset;

    void draw(GameRenderer* r) {
        glm::vec2 basis(offset);
        for (size_t i = 0; i < entries.size(); ++i) {
            bool active = false;
            if (activeEntry >= 0 && i == (unsigned)activeEntry) {
                active = true;
            }
            entries[i].draw(font, size, active, r, basis);
        }
    }

    void hover(const float x, const float y) {
        glm::vec2 c(x - offset.x, y - offset.y);
        for (size_t i = 0; i < entries.size(); ++i) {
            if (c.y > 0.f && c.y < size) {
                activeEntry = i;
                return;
            } else {
                c.y -= size;
            }
        }
    }

    void click(const float x, const float y) {
        glm::vec2 c(x - offset.x, y - offset.y);
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            if (c.y > 0.f && c.y < size) {
                (*it).activate(c.x, c.y);
                return;
            } else {
                c.y -= size;
            }
        }
    }

    // Activates the menu entry at the current active index.
    void activate() {
        if (activeEntry >= 0 && (unsigned)activeEntry < entries.size()) {
            entries[activeEntry].activate(0.f, 0.f);
        }
    }

    void move(int movement) {
        activeEntry += movement;
        if (activeEntry >= int(entries.size())) {
            activeEntry = 0;
        } else if (activeEntry < 0) {
            activeEntry = entries.size() - 1;
        }
    }

    const std::vector<MenuEntry>& getEntries() const {
        return entries;
    }

private:
    int font;
    float size;
    std::vector<MenuEntry> entries;
};

#endif
