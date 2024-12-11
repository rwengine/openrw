#include "loaders/LoaderTXD.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gl/gl_core_3_3.h"
#include "loaders/RWBinaryStream.hpp"
#include "platform/FileHandle.hpp"
#include "rw/debug.hpp"

namespace {
constexpr GLuint gErrorTextureData[] = {0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF};
constexpr GLuint gDebugTextureData[] = {0xFF0000FF, 0xFF00FF00};
constexpr GLuint gTextureRed[] = {0xFF0000FF};
constexpr GLuint gTextureGreen[] = {0xFF00FF00};
constexpr GLuint gTextureBlue[] = {0xFFFF0000};
}  // namespace

static
std::unique_ptr<TextureData> getErrorTexture() {
    GLuint errTexName = 0;
    std::unique_ptr<TextureData> tex = nullptr;
    if (errTexName == 0) {
        glGenTextures(1, &errTexName);
        glBindTexture(GL_TEXTURE_2D, errTexName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, gErrorTextureData);
        glGenerateMipmap(GL_TEXTURE_2D);

        tex = TextureData::create(errTexName, {2, 2}, false);
    }
    return tex;
}

const size_t paletteSize = 1024;

static
void processPalette(uint32_t* fullColor, RW::BinaryStreamSection& rootSection) {
    uint8_t* dataBase = reinterpret_cast<uint8_t*>(
        rootSection.raw() + sizeof(RW::BSSectionHeader) +
        sizeof(RW::BSTextureNative) - 4);

    uint8_t* coldata = (dataBase + paletteSize + sizeof(uint32_t));
    uint32_t raster_size = *reinterpret_cast<uint32_t*>(dataBase + paletteSize);
    uint32_t* palette = reinterpret_cast<uint32_t*>(dataBase);

    for (size_t j = 0; j < raster_size; ++j) {
        *(fullColor++) = palette[coldata[j]];
    }
}

static std::unique_ptr<TextureData> createTexture(
    RW::BSTextureNative& texNative, RW::BinaryStreamSection& rootSection) {
    // TODO: Exception handling.
    if (texNative.platform != 8) {
        RW_ERROR("Unsupported texture platform " << std::dec
                  << texNative.platform);
        return getErrorTexture();
    }

    bool isPal8 =
        (texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8) ==
        RW::BSTextureNative::FORMAT_EXT_PAL8;
    bool isFulc = texNative.rasterformat == RW::BSTextureNative::FORMAT_1555 ||
                  texNative.rasterformat == RW::BSTextureNative::FORMAT_8888 ||
                  texNative.rasterformat == RW::BSTextureNative::FORMAT_888;
    // Export this value
    bool transparent =
        !((texNative.rasterformat & RW::BSTextureNative::FORMAT_888) ==
          RW::BSTextureNative::FORMAT_888);

    if (texNative.dxttype) {
        RW_ERROR("FIXME: DXT format "
                 << static_cast<unsigned>(texNative.dxttype)
                 << " not yet implemented!");
        return getErrorTexture();
    }

    if (!(isPal8 || isFulc)) {
        RW_ERROR("Unsupported raster format " << std::dec
                  << texNative.rasterformat);
        return getErrorTexture();
    }

    GLuint textureName = 0;

    if (isPal8) {
        std::vector<uint32_t> fullColor(texNative.width * texNative.height);

        processPalette(fullColor.data(), rootSection);

        glGenTextures(1, &textureName);
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texNative.width,
                     texNative.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     fullColor.data());
    } else if (isFulc) {
        auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative);
        coldata += sizeof(uint32_t);

        GLenum type = GL_UNSIGNED_BYTE, format = GL_RGBA;
        switch (texNative.rasterformat) {
            case RW::BSTextureNative::FORMAT_1555:
                format = GL_RGBA;
                type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
                break;
            case RW::BSTextureNative::FORMAT_8888:
                format = GL_BGRA;
                // type = GL_UNSIGNED_INT_8_8_8_8_REV;
                coldata += 8;
                type = GL_UNSIGNED_BYTE;
                break;
            case RW::BSTextureNative::FORMAT_888:
                format = GL_BGRA;
                type = GL_UNSIGNED_BYTE;
                break;
            default:
                break;
        }

        glGenTextures(1, &textureName);
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texNative.width,
                     texNative.height, 0, format, type, coldata);
    } else {
        return getErrorTexture();
    }

    GLenum texFilter = GL_LINEAR;
    switch (texNative.filterflags & 0xFF) {
        default:
        case RW::BSTextureNative::FILTER_LINEAR:
            texFilter = GL_LINEAR;
            break;
        case RW::BSTextureNative::FILTER_NEAREST:
            texFilter = GL_NEAREST;
            break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texFilter);

    GLenum texwrap = GL_REPEAT;
    switch (texNative.wrapU) {
        default:
        case RW::BSTextureNative::WRAP_WRAP:
            texwrap = GL_REPEAT;
            break;
        case RW::BSTextureNative::WRAP_CLAMP:
            texwrap = GL_CLAMP_TO_EDGE;
            break;
        case RW::BSTextureNative::WRAP_MIRROR:
            texwrap = GL_MIRRORED_REPEAT;
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texwrap);

    switch (texNative.wrapV) {
        default:
        case RW::BSTextureNative::WRAP_WRAP:
            texwrap = GL_REPEAT;
            break;
        case RW::BSTextureNative::WRAP_CLAMP:
            texwrap = GL_CLAMP_TO_EDGE;
            break;
        case RW::BSTextureNative::WRAP_MIRROR:
            texwrap = GL_MIRRORED_REPEAT;
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texwrap);

    glGenerateMipmap(GL_TEXTURE_2D);

    return TextureData::create(textureName, {texNative.width, texNative.height},
                               transparent);
}

bool TextureLoader::loadFromMemory(const FileContentsInfo& file,
                                   TextureArchive& inTextures) {
    auto data = file.data.get();
    RW::BinaryStreamSection root(data);
    /*auto texDict =*/root.readStructure<RW::BSTextureDictionary>();

    size_t rootI = 0;
    while (root.hasMoreData(rootI)) {
        auto rootSection = root.getNextChildSection(rootI);

        if (rootSection.header.id != RW::SID_TextureNative) continue;

        RW::BSTextureNative texNative =
            rootSection.readStructure<RW::BSTextureNative>();
        std::string name = std::string(texNative.diffuseName);
        std::string alpha = std::string(texNative.alphaName);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(alpha.begin(), alpha.end(), alpha.begin(), ::tolower);

        inTextures[name] = createTexture(texNative, rootSection);
    }

    return true;
}
