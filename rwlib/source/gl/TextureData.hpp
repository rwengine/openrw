#ifndef _LIBRW_TEXTUREDATA_HPP_
#define _LIBRW_TEXTUREDATA_HPP_
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>

#include <memory>
#include <string>

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

    typedef std::shared_ptr<TextureData> Handle;

    static Handle create(GLuint name, const glm::ivec2& size,
                         bool transparent) {
        return std::make_shared<TextureData>(name, size, transparent);
    }

private:
    GLuint texName;
    glm::ivec2 size;
    bool hasAlpha;
};
using TextureArchive = std::map<std::string, TextureData::Handle>;

#endif
