#include "renderwure/render/DebugDraw.hpp"

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

DebugDraw::DebugDraw()
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
    glGenTextures(1, &texture);
    maxlines = 0;
}

DebugDraw::~DebugDraw()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vbo);
	glDeleteTextures(1, &texture);
}

void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    lines.push_back(glm::vec3(from.getX(), from.getY(), from.getZ()));
    lines.push_back(glm::vec3(to.getX(), to.getY(), to.getZ()));
    this->color = color;
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

void DebugDraw::drawAllLines()
{
    if(lines.size() == 0) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE0);
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

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if( lines.size() > maxlines ) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lines.size(), &lines[0].x, GL_STREAM_DRAW);
    }
    else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * lines.size(), &lines[0].x);
    }
    maxlines = std::max(maxlines, lines.size());

    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glm::mat4 model;
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_LINES, 0, lines.size());

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
