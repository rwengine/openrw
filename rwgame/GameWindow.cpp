#include <png.h>
#include <fstream>

#include <core/Logger.hpp>
#include "GameWindow.hpp"

GameWindow::GameWindow() :
	window(nullptr), glcontext(nullptr)
{

}


void GameWindow::create(size_t w, size_t h, bool fullscreen)
{
	uint32_t style = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	if (fullscreen)
		style |= SDL_WINDOW_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	window = SDL_CreateWindow("RWGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, style);
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


void GameWindow::captureToFile(const std::string& path, GameRenderer* renderer)
{
	Logger log;

	std::ofstream file(path);
	if (!file.is_open()) {
		log.error("Game", "Could not open screenshot file!");
		return;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		nullptr, nullptr, nullptr);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	png_set_write_fn(png_ptr, &file, {
		[](png_structp png_ptr, png_bytep data, png_size_t length) {
			reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr))->
				write(reinterpret_cast<char*>(data), length);
		} }, { [](png_structp) { } } );

	auto size = getSize();
	png_set_IHDR(png_ptr, info_ptr, size.x, size.y, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	unsigned char* buffer = renderer->getRenderer()->readPixels(size);
	for (int y = size.y-1; y >= 0; y--)
		png_write_row(png_ptr, &buffer[y * size.x * 3]);

	png_write_end(png_ptr, nullptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] buffer;
}


glm::ivec2 GameWindow::getSize() const
{
	int x, y;
	SDL_GL_GetDrawableSize(window, &x, &y);

	return glm::ivec2(x, y);
}
