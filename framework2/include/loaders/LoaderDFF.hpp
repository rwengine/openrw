#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

#include <vector>
#include <string>
#include <memory>

class Model;

class GTAData;

class LoaderDFF
{
private:
	template<class T> T readStructure(char *data, size_t &dataI);
	RW::BSSectionHeader readHeader(char *data, size_t &dataI);

public:
	Model* loadFromMemory(char *data, GTAData* gameData);
};

#endif
