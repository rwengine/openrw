#include <TextureArchive.hpp>

#include <cstring>
#include <iostream>
#include <cassert>

namespace RW
{
std::unique_ptr<TextureArchive> TextureArchive::create(BinaryStream &binaryStream)
{
  auto textureArchive = std::unique_ptr<TextureArchive>(new TextureArchive);

  auto section = binaryStream.rootHeader;

  assert(section->ID == BinaryStream::TEXTURE_DICTIONARY &&
         "BinaryStream passed to this function must be a TEXTURE DICTIONARY");

  // Struct
  section = section->next;
  textureArchive->numTextures = ((uint16_t *)section->data)[0];

  for (size_t i = 0; i < textureArchive->numTextures; i++) {
    section = section->next;  // Texture Native
    section = section->next;  // Struct

    Texture texture = *reinterpret_cast<Texture *>(section->data);

    if (texture.header.rasterFormat & 0x2000) {
      memcpy(texture.body.palette, section->data + sizeof(TextureHeader) - 4, 1024);

      texture.body.pixels = new uint8_t[texture.header.width * texture.header.height];
      memcpy(texture.body.pixels, section->data + sizeof(TextureHeader) + 1024,
             texture.header.width * texture.header.height);
    } else {
      size_t bufSize = texture.header.width * texture.header.height * texture.header.bpp / 8;
      texture.body.pixels = new uint8_t[bufSize];
      memcpy(texture.body.pixels, section->data + sizeof(TextureHeader), bufSize);
    }

    textureArchive->textures.push_back(texture);

    section = section->next;  // Extension
  }

  return textureArchive;
}
}
