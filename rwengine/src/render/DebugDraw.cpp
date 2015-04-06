#include "render/DebugDraw.hpp"

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

DebugDraw::DebugDraw()
: shaderProgram(nullptr)
{
	lineBuff = new GeometryBuffer;
	dbuff = new DrawBuffer;
	dbuff->setFaceType(GL_LINES);
	
    glGenTextures(1, &texture);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	int img = 0xFFFFFFFF;
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &img
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    maxlines = 0;
}

DebugDraw::~DebugDraw()
{
	delete dbuff;
	delete lineBuff;
}

void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
	btVector3 c = color * 255;
	lines.push_back({
		glm::vec3(from.getX(), from.getY(), from.getZ()),
		glm::vec3(0.f),
		glm::vec2(0.f),
		glm::u8vec4(c.getX(), c.getY(), c.getZ(), 255)
	});
	lines.push_back({
		glm::vec3(to.getX(), to.getY(), to.getZ()),
		glm::vec3(0.f),
		glm::vec2(0.f),
		glm::u8vec4(c.getX(), c.getY(), c.getZ(), 255)
	});
}

void DebugDraw::drawTriangle(const btVector3 &a, const btVector3 &b, const btVector3 &c, const btVector3 &color, btScalar alpha)
{
    /*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	float img[] = {color.getX(), color.getY(), color.getZ()};
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, 1, 1,
		0, GL_RGB, GL_FLOAT, img
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	GLint uniModel = glGetUniformLocation(shaderProgram, "model");

	float verts[] = {
		a.getX(), a.getY(), a.getZ(),
		b.getX(), b.getY(), b.getZ(),
		c.getX(), c.getY(), c.getZ(),
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::mat4 model;
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_TRIANGLES, 0, 3);*/
}

void DebugDraw::drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{

}

void DebugDraw::flush(GameRenderer* renderer)
{
	if(lines.size() == 0) {
		return;
	}
	
	renderer->getRenderer()->useProgram(shaderProgram);
	
	lineBuff->uploadVertices(lines);
	dbuff->addGeometry(lineBuff);

	Renderer::DrawParameters dp;
	dp.textures = {texture};
	dp.ambient = 1.f;
	dp.colour = glm::u8vec4(255, 255, 255, 255);
	dp.start = 0;
	dp.count = lines.size();
	dp.diffuse = 1.f;
	
	renderer->getRenderer()->drawArrays(glm::mat4(1.f), dbuff, dp);
	
	renderer->getRenderer()->invalidate();
	
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray( 0 );

	lines.clear();
}

void DebugDraw::reportErrorWarning(const char *warningString)
{
	std::cerr << warningString << std::endl;
}

void DebugDraw::draw3dText(const btVector3 &location, const char *textString)
{
	std::cout << textString << std::endl;
}

void DebugDraw::setDebugMode(int debugMode)
{
	this->debugMode = debugMode;
}

int DebugDraw::getDebugMode() const
{
	return debugMode;
}
