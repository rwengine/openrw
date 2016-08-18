#pragma once
#ifndef _TEXTUREARCHIVE_HPP_
#define _TEXTUREARCHIVE_HPP_

#include <BinaryStream.hpp>

#include <vector>

namespace RW
{
class TextureArchive
{
public:
  struct TextureHeader {
    uint32_t platformID;
    uint16_t filterFlags;
    uint8_t textureWrapV;
    uint8_t textureWrapU;
    uint8_t diffuseName[32];
    uint8_t alphaName[32];
    uint32_t rasterFormat;
    uint32_t alpha;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t numMipmap;
    uint8_t rasterType;
    uint8_t compression;
    uint32_t dataSize;
  };
  struct TextureBody {
    uint8_t palette[1024];
    uint8_t *pixels;
  };
  struct Texture {
    TextureHeader header;
    TextureBody body;
  };

  size_t numTextures;
  std::vector<Texture> textures;

  static std::unique_ptr<TextureArchive> create(BinaryStream &binaryStream);
};
}

#endif