#include "renderwure/render/Model.hpp"
#include <GL/glew.h>

Model::Geometry::Geometry()
	: VBO(0), EBO(0),
	flags(0)
{
	
}

Model::Geometry::~Geometry()
{
	if(VBO != 0) {
		glDeleteBuffers(1, &VBO);
	}
	if(EBO != 0) {
		glDeleteBuffers(1, &EBO);
	}
}

void Model::Geometry::generateNormals()
{
	
}
#include <iostream>

void Model::Geometry::buildBuffers()
{
	// OpenGL buffer stuff
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	size_t buffsize = (vertices.size() * sizeof(glm::vec3))
					+ texcoords.size() * sizeof(glm::vec2)
					+ normals.size() * sizeof(glm::vec3)
					+ colours.size() * sizeof(glm::vec4);

	// Vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, buffsize, NULL, GL_STATIC_DRAW);
	
	size_t dataOffset = 0;
	
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		(vertices.size() * sizeof(glm::vec3)),
		vertices.data()
	);
	offsVert = dataOffset;
	dataOffset += (vertices.size() * sizeof(glm::vec3));
	
	if(texcoords.size() > 0)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER,
			dataOffset,
			(texcoords.size() * sizeof(glm::vec2)),
			texcoords.data()
		);
		offsTexCoords = dataOffset;
		dataOffset += (texcoords.size() * sizeof(glm::vec2));
	}
	else {
		offsTexCoords = -1;
	}
	
	if(normals.size() > 0)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER,
			dataOffset,
			(normals.size() * sizeof(glm::vec3)),
			normals.data()
		);
		offsNormals = dataOffset;
		dataOffset += (normals.size() * sizeof(glm::vec3));
	}
	else {
		offsNormals = -1;
	}
	
	if(colours.size() > 0)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER,
			dataOffset,
			colours.size() * sizeof(glm::vec4),
			colours.data()
		);
		offsColours = dataOffset;
	}
	else {
		offsColours = -1;
	}
	
	size_t Ecount = 0;
	for(size_t i  = 0; i < subgeom.size(); ++i)
	{
		subgeom[i].start = Ecount;
		Ecount += subgeom[i].numIndices;
	}
	indicesCount = Ecount;

	// Allocate complete EBO buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * indicesCount,
				nullptr,
				GL_STATIC_DRAW);

	// Upload each subgeometry
	for(size_t i = 0; i < subgeom.size(); ++i)
	{
		glBufferSubData(
				GL_ELEMENT_ARRAY_BUFFER,
				subgeom[i].start * sizeof(uint32_t),
				sizeof(uint32_t) * subgeom[i].numIndices,
				subgeom[i].indices);
	}
}
