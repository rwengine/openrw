#ifndef _RWENGINE_DEBUGDRAW_HPP_
#define _RWENGINE_DEBUGDRAW_HPP_

#include <cstddef>
#include <vector>

#include <LinearMath/btIDebugDraw.h>
#include <LinearMath/btScalar.h>

#include <data/Clump.hpp>
#include <gl/gl_core_3_3.h>

#include "render/OpenGLRenderer.hpp"

class btVector3;
class DrawBuffer;
class GameRenderer;
class GeometryBuffer;

class DebugDraw : public btIDebugDraw {
public:
    DebugDraw();
    ~DebugDraw() override;

    void drawLine(const btVector3 &from, const btVector3 &to,
                  const btVector3 &color) override;
    void drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB,
                          btScalar distance, int lifeTime,
                          const btVector3 &color) override;
    void reportErrorWarning(const char *warningString) override;
    void draw3dText(const btVector3 &location, const char *textString) override;
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;

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

#endif
