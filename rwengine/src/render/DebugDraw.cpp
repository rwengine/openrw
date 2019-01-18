#include "render/DebugDraw.hpp"

#include <iostream>

#include <glm/glm.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <LinearMath/btVector3.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include <data/Clump.hpp>
#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>
#include <gl/gl_core_3_3.h>
#include <rw/debug.hpp>

#include "render/GameRenderer.hpp"

DebugDraw::DebugDraw() {
    dbuff->setFaceType(GL_LINES);

    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    int img = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    maxlines = 0;
}

void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to,
                         const btVector3 &color) {
    btVector3 c = color * 255;
    lines.emplace_back(glm::vec3(from.getX(), from.getY(), from.getZ()),
                     glm::vec3(0.f), glm::vec2(0.f),
                     glm::u8vec4(c.getX(), c.getY(), c.getZ(), 255));
    lines.emplace_back(glm::vec3(to.getX(), to.getY(), to.getZ()), glm::vec3(0.f),
                     glm::vec2(0.f),
                     glm::u8vec4(c.getX(), c.getY(), c.getZ(), 255));
}

void DebugDraw::drawContactPoint(const btVector3 &pointOnB,
                                 const btVector3 &normalOnB, btScalar distance,
                                 int lifeTime, const btVector3 &color) {
    RW_UNUSED(pointOnB);
    RW_UNUSED(normalOnB);
    RW_UNUSED(distance);
    RW_UNUSED(lifeTime);
    RW_UNUSED(color);
}

void DebugDraw::flush(GameRenderer &renderer) {
    if (lines.empty()) {
        return;
    }

    renderer.getRenderer().useProgram(shaderProgram);

    lineBuff->uploadVertices(lines);
    dbuff->addGeometry(lineBuff.get());

    Renderer::DrawParameters dp;
    dp.textures = {{texture}};
    dp.ambient = 1.f;
    dp.colour = glm::u8vec4(255, 255, 255, 255);
    dp.start = 0;
    dp.count = lines.size();
    dp.diffuse = 1.f;

    renderer.getRenderer().drawArrays(glm::mat4(1.f), dbuff.get(), dp);

    renderer.getRenderer().invalidate();

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    lines.clear();
}

void DebugDraw::reportErrorWarning(const char *warningString) {
    std::cerr << warningString << '\n';
}

void DebugDraw::draw3dText(const btVector3 &location, const char *textString) {
    RW_UNUSED(location);
    std::cout << textString << '\n';
}

void DebugDraw::setDebugMode(int debugMode) {
    this->debugMode = debugMode;
}

int DebugDraw::getDebugMode() const {
    return debugMode;
}
