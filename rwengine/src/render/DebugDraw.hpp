#ifndef _RWENGINE_DEBUGDRAW_HPP_
#define _RWENGINE_DEBUGDRAW_HPP_

#include <cstddef>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <LinearMath/btIDebugDraw.h>
#include <LinearMath/btScalar.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include "render/OpenGLRenderer.hpp"

class btVector3;
class DrawBuffer;
class GameRenderer;
class GeometryBuffer;
struct GeometryVertex;

class DebugDraw final : public btIDebugDraw {
public:
    DebugDraw();
    ~DebugDraw() override = default;

    void drawLine(const btVector3 &from, const btVector3 &to,
                  const btVector3 &color) override;
    void drawLine(const glm::vec3 &from, const glm::vec3 &to,
                  const glm::vec3 &color) {
        drawLine(btVector3{from.x, from.y, from.z},
                 btVector3{to.x, to.y, to.z},
                 btVector3{color.r, color.g, color.b});
    }
    void drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB,
                          btScalar distance, int lifeTime,
                          const btVector3 &color) override;
    void reportErrorWarning(const char *warningString) override;
    void draw3dText(const btVector3 &location, const char *textString) override;
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;

    void flush(GameRenderer &renderer);

protected:
    int debugMode;

    std::vector<GeometryVertex> lines;
    size_t maxlines;
    std::unique_ptr<GeometryBuffer> lineBuff = std::make_unique<GeometryBuffer>();
    std::unique_ptr<DrawBuffer> dbuff = std::make_unique<DrawBuffer>();

    GLuint texture;
};

#endif
