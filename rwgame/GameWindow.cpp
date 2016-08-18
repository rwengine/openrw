#include <core/Logger.hpp>
#include "GameWindow.hpp"

GameWindow::GameWindow() : window(nullptr), glcontext(nullptr) {}

void GameWindow::create(const std::string& title, size_t w, size_t h, bool fullscreen)
{
  uint32_t style = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if (fullscreen) style |= SDL_WINDOW_FULLSCREEN;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  window =
      SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, style);
  glcontext = SDL_GL_CreateContext(window);
}

void GameWindow::close()
{
  SDL_GL_DeleteContext(glcontext);
  SDL_DestroyWindow(window);

  window = nullptr;
}

void GameWindow::showCursor()
{
  SDL_SetRelativeMouseMode(SDL_FALSE);
  SDL_ShowCursor(SDL_ENABLE);
  SDL_SetWindowGrab(window, SDL_FALSE);
}

void GameWindow::hideCursor()
{
  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_ShowCursor(SDL_DISABLE);
  SDL_SetWindowGrab(window, SDL_TRUE);
}

glm::ivec2 GameWindow::getSize() const
{
  int x, y;
  SDL_GL_GetDrawableSize(window, &x, &y);

  return glm::ivec2(x, y);
}
