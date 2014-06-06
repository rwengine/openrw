#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

#include <vector>
#include <string>
#include <WorkContext.hpp>

class Model;

class GameData;

class LoaderDFF
{
private:
	template<class T> T readStructure(char *data, size_t &dataI);
	RW::BSSectionHeader readHeader(char *data, size_t &dataI);

public:
	Model* loadFromMemory(char *data, GameData* gameData);
};

#include <functional>

class LoadModelJob : public WorkJob
{
public:
	typedef std::function<void ( Model* )> ModelCallback;

private:
	GameData* _gameData;
	std::string _file;
	ModelCallback _callback;
	char* _data;
public:

	LoadModelJob(WorkContext* context, GameData* gd, const std::string& file, ModelCallback cb);

	void work();

	void complete();
};

#endif
