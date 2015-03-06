#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

#include <WorkContext.hpp>
#include <core/FileHandle.hpp>
#include <functional>
#include <string>
#include <map>

class GameData;

class TextureLoader
{
public:
	bool loadFromMemory(FileHandle file, GameData* gameData);
};

// TODO: refactor this interface to be more like ModelLoader so they can be rolled into one.
class LoadTextureArchiveJob : public WorkJob
{
private:
	GameData* _gameData;
	std::string _file;
	FileHandle data;
public:

	LoadTextureArchiveJob(WorkContext* context, GameData* gd, const std::string& file);

	void work();

	void complete();
};

#endif
