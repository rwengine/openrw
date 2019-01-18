#ifndef _LIBRW_TEXTUREDATA_HPP_
#define _LIBRW_TEXTUREDATA_HPP_

#include <gl/gl_core_3_3.h>
#include <glm/vec2.hpp>

#include <memory>
#include <string>
#include <unordered_map>

/**
 * Stores a handle and metadata about a loaded texture.
 */
class TextureData {
public:
    TextureData(GLuint name, const glm::ivec2& dims, bool alpha)
        : texName(name), size(dims), hasAlpha(alpha) {
    }

    ~TextureData() {
        glDeleteTextures(1, &texName);
    }

    GLuint getName() const {
        return texName;
    }

    const glm::ivec2& getSize() const {
        return size;
    }

    bool isTransparent() const {
        return hasAlpha;
    }

    static auto create(GLuint name, const glm::ivec2& size,
                         bool transparent) {
        return std::make_unique<TextureData>(name, size, transparent);
    }

private:
    GLuint texName;
    glm::ivec2 size;
    bool hasAlpha;
};
using TextureArchive = std::unordered_map<std::string, std::unique_ptr<TextureData>>;

#endif
