#include "MenuSystem.hpp"

#include <render/GameRenderer.hpp>

void Menu::MenuEntry::draw(font_t font, float size, bool active,
                           GameRenderer &r, glm::vec2 &basis) {
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
    r.text.renderText(ti);
    basis.y += size;
}

void Menu::draw(GameRenderer &r) {
    glm::vec2 basis(offset);
    for (size_t i = 0; i < entries.size(); ++i) {
        bool active = false;
        if (activeEntry >= 0 && i == static_cast<unsigned>(activeEntry)) {
            active = true;
        }
        entries[i].draw(font, size, active, r, basis);
    }
}

void Menu::hover(const float x, const float y) {
    glm::vec2 c(x - offset.x, y - offset.y);
    for (size_t i = 0; i < entries.size(); ++i) {
        if (c.y > 0.f && c.y < size) {
            activeEntry = static_cast<int>(i);
            return;
        } else {
            c.y -= size;
        }
    }
}

void Menu::click(const float x, const float y) {
    glm::vec2 c(x - offset.x, y - offset.y);
    for (auto &entry : entries) {
        if (c.y > 0.f && c.y < size) {
            entry.activate(c.x, c.y);
            return;
        } else {
            c.y -= size;
        }
    }
}

void Menu::activate() {
    if (activeEntry >= 0 &&
        static_cast<unsigned>(activeEntry) < entries.size()) {
        entries[activeEntry].activate(0.f, 0.f);
    }
}

void Menu::move(int movement) {
    activeEntry += movement;
    if (activeEntry >= static_cast<int>(entries.size())) {
        activeEntry = 0;
    } else if (activeEntry < 0) {
        activeEntry = static_cast<int>(entries.size() - 1);
    }
}
