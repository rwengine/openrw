#pragma once
#include <gl/gl_core_3_3.h>
#include <glm/glm.hpp>

#include <memory>

/**
 * Stores a handle and metadata about a loaded texture.
 */
class TextureData
{
public:
  TextureData(GLuint name, const glm::ivec2& dims, bool alpha)
      : texName(name), size(dims), hasAlpha(alpha)
  {
  }

  GLuint getName() const { return texName; }

  const glm::ivec2& getSize() const { return size; }

  bool isTransparent() const { return hasAlpha; }

  typedef std::shared_ptr<TextureData> Handle;

  static Handle create(GLuint name, const glm::ivec2& size, bool transparent)
  {
    return Handle(new TextureData(name, size, transparent));
  }

private:
  GLuint texName;
  glm::ivec2 size;
  bool hasAlpha;
};
