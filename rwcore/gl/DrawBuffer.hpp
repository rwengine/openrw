#ifndef _LIBRW_DRAWBUFFER_HPP_
#define _LIBRW_DRAWBUFFER_HPP_
#include <gl/gl_core_3_3.h>

class GeometryBuffer;

/**
 * DrawBuffer stores VAO state
 */
class DrawBuffer {
    GLuint vao = 0u;

    GLenum facetype{};

public:
    DrawBuffer() = default;
    ~DrawBuffer();

    GLuint getVAOName() const {
        return vao;
    }

    void setFaceType(GLenum ft) {
        facetype = ft;
    }

    GLenum getFaceType() const {
        return facetype;
    }

    /**
     * Adds a Geometry Buffer to the Draw Buffer.
     */
    void addGeometry(GeometryBuffer* gbuff);
};

#endif
