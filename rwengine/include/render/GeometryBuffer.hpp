#pragma once
#ifndef _GEOMETRYBUFFER_HPP_
#define _GEOMETRYBUFFER_HPP_
#include <GL/glew.h>
#include <vector>

/**
 * Enum used to determine which shader input an attribute maps to
 */
enum AttributeSemantic {
	ATRS_Position,
	ATRS_Normal,
	ATRS_Colour,
	ATRS_TexCoord
};

/**
 * Stores Vertex Attribute data
 */
struct AttributeIndex {
	AttributeSemantic sem;
	GLsizei size;
	/*GLenum type*/
	GLsizei stride;
	GLsizei offset;
};

typedef std::vector<AttributeIndex> AttributeList;

class GeometryBuffer {
	GLuint vbo;
	GLsizei num;
	
	AttributeList attributes;
public:

	GeometryBuffer();
	~GeometryBuffer();

	GLuint getVBOName() const
		{ return vbo; }
	
	/**
	 * Uploads Vertex Buffer data from an STL vector
	 */
	template<class T> void uploadVertices(const std::vector<T>& data) {
		uploadVertices(data.size(), data.size()*sizeof(T), data.data());
		// Assume T has a static method for attributes;
		attributes = T::vertex_attributes();
	}
	
	/**
	 * Uploads raw memory into the buffer.
	 */
	void uploadVertices(GLsizei num, GLsizeiptr size, const GLvoid* mem);
	
	const AttributeList& getDataAttributes() const 
		{ return attributes; }
};

#endif