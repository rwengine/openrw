#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <string>
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

#include <render/GameRenderer.hpp>


class GameWindow
{
	SDL_Window* window;
	SDL_GLContext glcontext;

public:
	GameWindow();

	void create(size_t w, size_t h, bool fullscreen);
	void close();

	void showCursor();
	void hideCursor();

	glm::ivec2 getSize() const;

	void swap() const
	{
		SDL_GL_SwapWindow(window);
	}

	bool isOpen() const
	{
		return !!window;
	}
};

#endif
