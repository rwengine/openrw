#pragma once

#include <LinearMath/btIDebugDraw.h>
#include <data/Clump.hpp>
#include <render/GameRenderer.hpp>

class DebugDraw : public btIDebugDraw {
public:
    DebugDraw();
    ~DebugDraw();

    void drawLine(const btVector3 &from, const btVector3 &to,
                  const btVector3 &color);
    void drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB,
                          btScalar distance, int lifeTime,
                          const btVector3 &color);
    void reportErrorWarning(const char *warningString);
    void draw3dText(const btVector3 &location, const char *textString);
    void setDebugMode(int debugMode);
    int getDebugMode() const;

    void flush(GameRenderer *renderer);

    void setShaderProgram(Renderer::ShaderProgram *shaderProgram) {
        this->shaderProgram = shaderProgram;
    }

protected:
    int debugMode;

    std::vector<GeometryVertex> lines;
    size_t maxlines;
    GeometryBuffer *lineBuff;
    DrawBuffer *dbuff;

    //Ownership is handled by worldProg in renderer
    Renderer::ShaderProgram *shaderProgram;

    GLuint texture;
};
