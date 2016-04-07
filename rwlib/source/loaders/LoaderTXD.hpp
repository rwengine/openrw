#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#include <loaders/RWBinaryStream.hpp>

#include <job/WorkContext.hpp>
#include <platform/FileHandle.hpp>
#include <functional>
#include <string>
#include <map>

// This might suffice
#include <gl/TextureData.hpp>
typedef std::map<std::pair<std::string, std::string>, TextureData::Handle> TextureArchive;

class FileIndex;

class TextureLoader
{
public:
	bool loadFromMemory(FileHandle file, TextureArchive& inTextures);
};

// TODO: refactor this interface to be more like ModelLoader so they can be rolled into one.
class LoadTextureArchiveJob : public WorkJob
{
private:
	TextureArchive& archive;
	FileIndex* fileIndex;
	std::string _file;
	FileHandle data;
public:

	LoadTextureArchiveJob(WorkContext* context, FileIndex* index, TextureArchive& inTextures, const std::string& file);

	void work();

	void complete();
};

#endif
