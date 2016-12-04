#pragma once
#ifndef _TEXTURELOADER_HPP_
#define _TEXTURELOADER_HPP_

#include <loaders/RWBinaryStream.hpp>

#include <functional>
#include <map>
#include <platform/FileHandle.hpp>
#include <string>

// This might suffice
#include <gl/TextureData.hpp>

class FileIndex;

class TextureLoader {
public:
    bool loadFromMemory(FileHandle file, TextureArchive& inTextures);
};

#endif
