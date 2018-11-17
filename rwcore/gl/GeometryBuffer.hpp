#ifndef _LIBRW_GEOMETRYBUFFER_HPP_
#define _LIBRW_GEOMETRYBUFFER_HPP_
#include <gl/gl_core_3_3.h>
#include <vector>

/**
 * Enum used to determine which shader input an attribute maps to
 */
enum AttributeSemantic {
    ATRS_Position = 0,
    ATRS_Normal = 1,
    ATRS_Colour = 2,
    ATRS_TexCoord = 3
};

/**
 * Stores Vertex Attribute data
 */
struct AttributeIndex {
    AttributeSemantic sem;
    GLsizei size;
    GLsizei stride;
    size_t offset;
    GLenum type;

    AttributeIndex(AttributeSemantic s, GLsizei sz, GLsizei strd, size_t offs,
                   GLenum type = GL_FLOAT)
        : sem(s), size(sz), stride(strd), offset(offs), type(type) {
    }
};

typedef std::vector<AttributeIndex> AttributeList;

/**
 * GeometryBuffer stores a set of vertex attribute data
 */
class GeometryBuffer {
    GLuint vbo = 0;
    GLsizei num = 0;

    AttributeList attributes{};

public:
    GeometryBuffer() = default;
    template<class T>
    GeometryBuffer(const std::vector<T>& data) {
        uploadVertices(data);
    }

    ~GeometryBuffer();

    GLuint getVBOName() const {
        return vbo;
    }

    GLsizei getCount() const {
        return num;
    }

    /**
     * Uploads Vertex Buffer data from an STL vector
     *
     * vertex_attributes() is assumed to exist so that vertex types
     * can implicitly declare the strides and offsets for their data.
     */
    template<class T>
    void uploadVertices(const std::vector<T>& data) {
        uploadVertices(static_cast<GLsizei>(data.size()),
                       data.size() * sizeof(T), data.data());
        // Assume T has a static method for attributes;
        attributes = T::vertex_attributes();
    }

    /**
     * Uploads raw memory into the buffer.
     */
    void uploadVertices(GLsizei num, GLsizeiptr size, const GLvoid* mem);

    const AttributeList& getDataAttributes() const {
        return attributes;
    }
    AttributeList& getDataAttributes() {
        return attributes;
    }
};

#endif
