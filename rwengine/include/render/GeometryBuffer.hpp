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
	GLsizei stride;
	GLsizei offset;
	GLenum type;

	AttributeIndex(AttributeSemantic s,
				   GLsizei sz,
				   GLsizei strd,
				   GLsizei offs,
				   GLenum type = GL_FLOAT)
			: sem(s), size(sz), stride(strd), offset(offs), type(type)
	{}
};

typedef std::vector<AttributeIndex> AttributeList;

/** 
 * GeometryBuffer stores a set of vertex attribute data
 */
class GeometryBuffer {
	GLuint vbo;
	GLsizei num;
	
	AttributeList attributes;
public:

	GeometryBuffer();
	template<class T> GeometryBuffer(const std::vector<T>& data)
		: vbo(0), num(0)
	{
		uploadVertices(data);
	}

	~GeometryBuffer();

	GLuint getVBOName() const
		{ return vbo; }
	
	GLsizei getCount() const
		{ return num; }
	
	/**
	 * Uploads Vertex Buffer data from an STL vector
	 * 
	 * vertex_attributes() is assumed to exist so that vertex types
	 * can implicitly declare the strides and offsets for their data.
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
