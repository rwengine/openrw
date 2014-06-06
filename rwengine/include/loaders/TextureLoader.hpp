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

#include <WorkContext.hpp>
#include <functional>

// TODO: refactor this interface to be more like ModelLoader so they can be rolled into one.
class LoadTextureArchiveJob : public WorkJob
{
private:
	GameData* _gameData;
	std::string _file;
	char* _data;
public:

	LoadTextureArchiveJob(WorkContext* context, GameData* gd, const std::string& file);

	void work();

	void complete();
};

#endif
