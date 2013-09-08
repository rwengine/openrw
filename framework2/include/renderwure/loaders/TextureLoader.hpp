#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <renderwure/loaders/rwbinarystream.h>

#include <string>
#include <map>

class TextureLoader
{
public:
	bool loadFromFile(std::string filename);
	bool loadFromMemory(char *data);
	void bindTexture(std::string texture);

	std::map<std::string, GLuint> textures;
};

#endif
