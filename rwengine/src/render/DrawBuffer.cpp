#include <render/DrawBuffer.hpp>
#include <render/GeometryBuffer.hpp>
#include <map>

/* TODO: Come up with a more elegant solution to "WHICH ARRAY IS IT?" */
std::map<AttributeSemantic, GLuint> semantic_to_attrib_array = {
	{ATRS_Position, 0},
	{ATRS_Normal, 4},
	{ATRS_Colour, 8},
	{ATRS_TexCoord, 12}
};

DrawBuffer::DrawBuffer()
: vao(0)
{

}

DrawBuffer::~DrawBuffer()
{
	if(vao) {
		glDeleteVertexArrays(1, &vao);
	}
}

void DrawBuffer::addGeometry(GeometryBuffer* gbuff)
{
	if(vao == 0) {
		glGenVertexArrays(1, &vao);
	}
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, gbuff->getVBOName());
	// Iterate the attributes present in the gbuff
	for(const AttributeIndex& at : gbuff->getDataAttributes()) {
		GLuint vaoindex = semantic_to_attrib_array[at.sem];
		glEnableVertexAttribArray(vaoindex);
		glVertexAttribPointer(vaoindex, at.size, GL_FLOAT, GL_FALSE, at.stride, reinterpret_cast<GLvoid*>(at.offset));
	}
}
