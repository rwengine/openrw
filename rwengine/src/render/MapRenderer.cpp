#include "render/MapRenderer.hpp"

#include <cstdint>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <gl/gl_core_3_3.h>
#include <gl/TextureData.hpp>

#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "objects/GameObject.hpp"

const char* MapVertexShader = R"(
#version 330

layout(location = 0) in vec2 position;
out vec2 TexCoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
	// UI space is top to bottom, so flip Y.
	TexCoord = position + vec2( 0.5 );
})";

const char* MapFragmentShader = R"(
#version 330

in vec2 TexCoord;
uniform vec4 colour;
uniform sampler2D spriteTexture;
out vec4 outColour;

void main()
{
	vec4 c = texture(spriteTexture, TexCoord*0.99);
	outColour = vec4(colour.rgb + c.rgb, colour.a * c.a);
})";

MapRenderer::MapRenderer(std::shared_ptr<Renderer> renderer, GameData* _data)
    : data(_data), renderer(renderer) {
    rectGeom.uploadVertices<VertexP2>(
        {{-.5f, -.5f}, {.5f, -.5f}, {.5f, .5f}, {-.5f, .5f}});
    rect.addGeometry(&rectGeom);
    rect.setFaceType(GL_TRIANGLE_FAN);

    std::vector<VertexP2> circleVerts;
    circleVerts.emplace_back(0.f, 0.f);
    for (int v = 0; v < 181; ++v) {
        circleVerts.emplace_back(
             0.5f * glm::cos(2 * (v / 180.f) * glm::pi<float>()),
             0.5f * glm::sin(2 * (v / 180.f) * glm::pi<float>()));
    }
    circleGeom.uploadVertices(circleVerts);
    circle.addGeometry(&circleGeom);
    circle.setFaceType(GL_TRIANGLE_FAN);

    rectProg = renderer->createShader(MapVertexShader, MapFragmentShader);

    renderer->setUniform(rectProg.get(), "colour", glm::vec4(1.f));
}

#define GAME_MAP_SIZE 4000

void MapRenderer::draw(GameWorld* world, const MapInfo& mi) {
    renderer->pushDebugGroup("Map");
    renderer->useProgram(rectProg.get());

    Renderer::DrawParameters dp { };
    dp.start = 0;
    dp.blend = true;
    dp.depthWrite = false;

    // World out the number of units per tile
    glm::vec2 worldSize(GAME_MAP_SIZE);
    const int mapBlockLine = 8;
    glm::vec2 tileSize = worldSize / (float)mapBlockLine;
    // Determine the scale to show the right number of world units on the screen
    float worldScale = mi.screenSize / mi.worldSize;

    auto proj = renderer->get2DProjection();
    glm::mat4 view, model;
    renderer->setUniform(rectProg.get(), "proj", proj);
    renderer->setUniform(rectProg.get(), "model", glm::mat4());
    renderer->setUniform(rectProg.get(), "colour", glm::vec4(0.f, 0.f, 0.f, 1.f));

    view = glm::translate(view, glm::vec3(mi.screenPosition, 0.f));

    if (mi.clipToSize) {
        glm::mat4 circleView = glm::scale(view, glm::vec3(mi.screenSize));
        renderer->setUniform(rectProg.get(), "view", circleView);
        dp.count = 182;
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glColorMask(0x00, 0x00, 0x00, 0x00);
        renderer->drawArrays(glm::mat4(), &circle, dp);
        glColorMask(0xFF, 0xFF, 0xFF, 0xFF);
        glStencilFunc(GL_EQUAL, 1, 0xFF);
    }

    view = glm::scale(view, glm::vec3(worldScale));
    view = glm::rotate(view, mi.rotation, glm::vec3(0.f, 0.f, 1.f));
    view = glm::translate(
        view, glm::vec3(glm::vec2(-1.f, 1.f) * mi.worldCenter, 0.f));
    renderer->setUniform(rectProg.get(), "view", view);

    // radar00 = -x, +y
    // incrementing in X, then Y

    int initX = -(mapBlockLine / 2);
    int initY = -(mapBlockLine / 2);

    for (int m = 0; m < MAP_BLOCK_SIZE; ++m) {
        std::string num = (m < 10 ? "0" : "");
        std::string name = "radar" + num + std::to_string(m);
        auto texture = world->data->findSlotTexture(name, name);
        dp.textures = {texture->getName()};

        dp.count = 4;

        int mX = initX + (m % mapBlockLine);
        int mY = initY + (m / mapBlockLine);

        auto tc = glm::vec2(mX, mY) * tileSize + glm::vec2(tileSize / 2.f);

        glm::mat4 tilemodel = model;
        tilemodel = glm::translate(tilemodel, glm::vec3(tc, 0.f));
        tilemodel = glm::scale(tilemodel, glm::vec3(tileSize, 1.f));

        renderer->setUniform(rectProg.get(), "model", tilemodel);

        renderer->drawArrays(glm::mat4(), &rect, dp);
    }

    // From here on out we will work in screenspace
    renderer->setUniform(rectProg.get(), "view", glm::mat4());

    if (mi.clipToSize) {
        glDisable(GL_STENCIL_TEST);
        // We only need the outer ring if we're clipping.
        glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ZERO);
        TextureData::Handle radarDisc =
            data->findSlotTexture("hud", "radardisc");
        dp.textures = {radarDisc->getName()};

        glm::mat4 model;
        model = glm::translate(model, glm::vec3(mi.screenPosition, 0.0f));
        model = glm::scale(model, glm::vec3(mi.screenSize * 1.07f));
        renderer->setUniform(rectProg.get(), "model", model);
        renderer->drawArrays(glm::mat4(), &rect, dp);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                            GL_ZERO);
    }

    // Draw the player blip
    auto player = world->pedestrianPool.find(world->state->playerObject);
    if (player) {
        glm::vec2 plyblip(player->getPosition());
        float hdg = glm::roll(player->getRotation());
        drawBlip(plyblip, view, mi, "radar_centre",
                 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 18.0f, mi.rotation - hdg);
    }

    drawBlip(mi.worldCenter + glm::vec2(0.f, mi.worldSize), view, mi,
             "radar_north", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 24.f);

    for (auto& radarBlip : world->state->radarBlips) {
        const auto& blip = radarBlip.second;

        auto dm = blip.display;
        if (dm == BlipData::Hide || dm == BlipData::MarkerOnly) {
            continue;
        }

        glm::vec2 blippos(blip.coord);
        if (blip.target > 0) {
            GameObject* object = world->getBlipTarget(blip);
            if (object) {
                blippos = glm::vec2(object->getPosition());
            }
        }

        const auto& texture = blip.texture;
        if (!texture.empty()) {
            drawBlip(blippos, view, mi, texture,
                     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 18.0f);
        } else {
            // Colours from http://www.gtamodding.com/wiki/0165 (colors not
            // specific to that opcode!)
            uint32_t rgbaValue;
            switch (blip.colour) {
                case 0:  // RED
                    rgbaValue = blip.dimmed ? 0x7F0000FF : 0x712B49FF;
                    break;
                case 1:  // GREEN
                    rgbaValue = blip.dimmed ? 0x007F00FF : 0x5FA06AFF;
                    break;
                case 2:  // BLUE
                    rgbaValue = blip.dimmed ? 0x00007FFF : 0x80A7F3FF;
                    break;
                case 3:  // WHITE
                    rgbaValue = blip.dimmed ? 0x7F7F7FFF : 0xE1E1E1FF;
                    break;
                case 4:  // YELLOW
                    rgbaValue = blip.dimmed ? 0x7F7F00FF : 0xFFFF00FF;
                    break;
                case 5:  // PURPLE
                    rgbaValue = blip.dimmed ? 0x7F007FFF : 0xFF00FFFF;
                    break;
                case 6:  // CYAN
                    rgbaValue = blip.dimmed ? 0x007F7FFF : 0x00FFFFFF;
                    break;
                default:  // Extended mode (Dimming ignored)
                    rgbaValue = blip.colour;
                    break;
            }

            glm::vec4 colour((rgbaValue >> 24) / 255.0f,
                             ((rgbaValue >> 16) & 0xFF) / 255.0f,
                             ((rgbaValue >> 8) & 0xFF) / 255.0f,
                             1.0f  // Note: Alpha is not controlled by blip
                             );

            drawBlip(blippos, view, mi, colour, blip.size * 2.0f);
        }
    }

    /// @TODO migrate to using the renderer
    renderer->invalidate();
    renderer->popDebugGroup();
}

void MapRenderer::prepareBlip(const glm::vec2& coord, const glm::mat4& view,
                              const MapInfo& mi, const std::string& texture,
                              glm::vec4 colour, float size, float heading) {
    glm::vec2 adjustedCoord = coord;
    if (mi.clipToSize) {
        float maxDist = mi.worldSize / 2.f;
        float centerDist = glm::distance(coord, mi.worldCenter);
        if (centerDist > maxDist) {
            adjustedCoord = mi.worldCenter +
                            ((coord - mi.worldCenter) / centerDist) * maxDist;
        }
    }

    glm::vec3 viewPos(
        view * glm::vec4(glm::vec2(1.f, -1.f) * adjustedCoord, 0.f, 1.f));
    glm::mat4 model;
    model = glm::translate(model, viewPos);
    model = glm::scale(model, glm::vec3(size));
    model = glm::rotate(model, heading, glm::vec3(0.f, 0.f, 1.f));
    renderer->setUniform(rectProg.get(), "model", model);

    GLuint tex = 0;
    if (!texture.empty()) {
        auto sprite = data->findSlotTexture("hud", texture);
        tex = sprite->getName();
    }
    renderer->setUniform(rectProg.get(), "colour", colour);

    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void MapRenderer::drawBlip(const glm::vec2& coord, const glm::mat4& view,
                           const MapInfo& mi, const std::string& texture,
                           glm::vec4 colour, float size, float heading) {
    prepareBlip(coord, view, mi, texture, colour, size, heading);
    glBindVertexArray(rect.getVAOName());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void MapRenderer::drawBlip(const glm::vec2& coord, const glm::mat4& view,
                           const MapInfo& mi, glm::vec4 colour, float size) {
    drawBlip(coord, view, mi, "", colour, size);
    // Draw outline
    renderer->setUniform(rectProg.get(), "colour", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}
