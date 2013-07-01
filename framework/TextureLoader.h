#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include "rwbinarystream.h"

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
