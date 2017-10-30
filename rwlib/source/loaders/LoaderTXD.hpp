#ifndef _LIBRW_TEXTURELOADER_HPP_
#define _LIBRW_TEXTURELOADER_HPP_

#include <gl/TextureData.hpp>
#include <rw/forward.hpp>

class TextureLoader {
public:
    bool loadFromMemory(FileHandle file, TextureArchive& inTextures);
};

#endif
