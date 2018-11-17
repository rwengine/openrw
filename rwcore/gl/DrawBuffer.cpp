#include "gl/DrawBuffer.hpp"

#include <map>

#include <gl/gl_core_3_3.h>
#include <gl/GeometryBuffer.hpp>

DrawBuffer::DrawBuffer() : vao(0) {
}

DrawBuffer::~DrawBuffer() {
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
}

void DrawBuffer::addGeometry(GeometryBuffer* gbuff) {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, gbuff->getVBOName());
    // Iterate the attributes present in the gbuff
    for (const AttributeIndex& at : gbuff->getDataAttributes()) {
        auto vaoindex = static_cast<GLuint>(at.sem);
        glEnableVertexAttribArray(vaoindex);
        glVertexAttribPointer(vaoindex, static_cast<GLint>(at.size), at.type,
                              GL_TRUE, at.stride,
                              reinterpret_cast<void*>(at.offset));
    }
}
