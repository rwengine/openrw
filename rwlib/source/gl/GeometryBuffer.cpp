#include "gl/GeometryBuffer.hpp"

GeometryBuffer::GeometryBuffer() : vbo(0), num(0) {
}

GeometryBuffer::~GeometryBuffer() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }
}

void GeometryBuffer::uploadVertices(GLsizei num, GLsizeiptr size,
                                    const GLvoid* mem) {
    if (vbo == 0) {
        glGenBuffers(1, &vbo);
    }
    this->num = num;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, mem, GL_STATIC_DRAW);
}
