#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

#include <string>
#include <map>

class GameData;

class TextureLoader
{
public:
	bool loadFromFile(std::string filename, GameData* gameData);
	bool loadFromMemory(char *data, GameData* gameData);
};

#endif
