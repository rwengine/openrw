#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <SDL.h>
#include <glm/vec2.hpp>
#include <string>

#include <render/GameRenderer.hpp>

class GameWindow {
    SDL_Window* window = nullptr;
    SDL_Surface* icon = nullptr;
    SDL_GLContext glcontext{nullptr};

public:
    GameWindow() = default;

    void create(const std::string& title, size_t w, size_t h, bool fullscreen);
    void close();

    void showCursor();
    void hideCursor();

    glm::ivec2 getSize() const;

    void swap() const {
        SDL_GL_SwapWindow(window);
    }

    bool isOpen() const {
        return !!window;
    }
};

#endif
