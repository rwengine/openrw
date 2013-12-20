#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

#include <string>
#include <map>

class GTAData;

class TextureLoader
{
public:
	bool loadFromFile(std::string filename, GTAData* gameData);
	bool loadFromMemory(char *data, GTAData* gameData);
};

#endif
