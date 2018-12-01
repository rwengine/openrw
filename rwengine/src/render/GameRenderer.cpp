#include "render/GameRenderer.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <gl/TextureData.hpp>
#include <rw/types.hpp>

#include "core/Logger.hpp"
#include "core/Profiler.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "loaders/WeatherLoader.hpp"
#include "objects/GameObject.hpp"
#include "render/ObjectRenderer.hpp"
#include "render/GameShaders.hpp"
#include "render/VisualFX.hpp"

constexpr size_t skydomeSegments = 8, skydomeRows = 10;

/// @todo collapse all of these into "VertPNC" etc.
struct ParticleVert {
    static const AttributeList vertex_attributes() {
        return {{ATRS_Position, 2, sizeof(ParticleVert), 0ul},
                {ATRS_TexCoord, 2, sizeof(ParticleVert), 2ul * sizeof(float)},
                {ATRS_Colour, 3, sizeof(ParticleVert), 4ul * sizeof(float)}};
    }

    float x, y;
    float u, v;
    float r, g, b;
};

GameRenderer::GameRenderer(Logger* log, GameData* _data)
    : data(_data)
    , logger(log)
    , map(*renderer, _data)
    , water(*this)
    , text(*this) {
    logger->info("Renderer", renderer->getIDString());

    worldProg =
        renderer->createShader(GameShaders::WorldObject::VertexShader,
                               GameShaders::WorldObject::FragmentShader);

    renderer->setUniformTexture(worldProg.get(), "texture", 0);
    renderer->setProgramBlockBinding(worldProg.get(), "SceneData", 1);
    renderer->setProgramBlockBinding(worldProg.get(), "ObjectData", 2);

    particleProg =
        renderer->createShader(GameShaders::WorldObject::VertexShader,
                               GameShaders::Particle::FragmentShader);

    renderer->setUniformTexture(particleProg.get(), "texture", 0);
    renderer->setProgramBlockBinding(particleProg.get(), "SceneData", 1);
    renderer->setProgramBlockBinding(particleProg.get(), "ObjectData", 2);

    skyProg = renderer->createShader(GameShaders::Sky::VertexShader,
                                     GameShaders::Sky::FragmentShader);

    renderer->setProgramBlockBinding(skyProg.get(), "SceneData", 1);

    postProg =
        renderer->createShader(GameShaders::DefaultPostProcess::VertexShader,
                               GameShaders::DefaultPostProcess::FragmentShader);

    glGenVertexArrays(1, &vao);

    glGenFramebuffers(1, &framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
    glGenTextures(2, fbTextures);

    glBindTexture(GL_TEXTURE_2D, fbTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, fbTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 128, 128, 0, GL_RED, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           fbTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           fbTextures[1], 0);

    // Give water renderer the data texture
    water.setDataTexture(1, fbTextures[1]);

    glGenRenderbuffers(1, fbRenderBuffers);
    glBindRenderbuffer(GL_RENDERBUFFER, fbRenderBuffers[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 128, 128);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, fbRenderBuffers[0]);

    // Create the skydome

    size_t segments = skydomeSegments, rows = skydomeRows;

    float R = 1.f / static_cast<float>(rows - 1);
    float S = 1.f / static_cast<float>(segments - 1);
    std::vector<VertexP3> skydomeVerts;
    skydomeVerts.resize(rows * segments);
    for (size_t r = 0, i = 0; r < rows; ++r) {
        for (size_t s = 0; s < segments; ++s) {
            skydomeVerts[i++].position =
                glm::vec3(cos(2.f * glm::pi<float>() * s * S) *
                              cos(glm::half_pi<float>() * r * R),
                          sin(2.f * glm::pi<float>() * s * S) *
                              cos(glm::half_pi<float>() * r * R),
                          sin(glm::half_pi<float>() * r * R));
        }
    }
    skyGbuff.uploadVertices(skydomeVerts);
    skyDbuff.addGeometry(&skyGbuff);
    skyDbuff.setFaceType(GL_TRIANGLES);

    glGenBuffers(1, &skydomeIBO);
    std::vector<GLuint> skydomeIndBuff;
    skydomeIndBuff.resize(rows * segments * 6);
    for (size_t r = 0, i = 0; r < (rows - 1); ++r) {
        for (size_t s = 0; s < (segments - 1); ++s) {
            skydomeIndBuff[i++] = static_cast<GLuint>(r * segments + s);
            skydomeIndBuff[i++] = static_cast<GLuint>(r * segments + (s + 1));
            skydomeIndBuff[i++] = static_cast<GLuint>((r + 1) * segments + (s + 1));
            skydomeIndBuff[i++] = static_cast<GLuint>(r * segments + s);
            skydomeIndBuff[i++] = static_cast<GLuint>((r + 1) * segments + (s + 1));
            skydomeIndBuff[i++] = static_cast<GLuint>((r + 1) * segments + s);
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skydomeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(GLuint) * skydomeIndBuff.size(), skydomeIndBuff.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);

    particleGeom.uploadVertices<ParticleVert>(
        {{0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f},
         {-0.5f, 0.5f, 0.f, 1.f, 1.f, 1.f, 1.f},
         {0.5f, -0.5f, 1.f, 0.f, 1.f, 1.f, 1.f},
         {-0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f}});
    particleDraw.addGeometry(&particleGeom);
    particleDraw.setFaceType(GL_TRIANGLE_STRIP);

    ssRectGeom.uploadVertices<VertexP2>({{-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}});
    ssRectDraw.addGeometry(&ssRectGeom);
    ssRectDraw.setFaceType(GL_TRIANGLE_STRIP);

    ssRectProg =
        renderer->createShader(GameShaders::ScreenSpaceRect::VertexShader,
                               GameShaders::ScreenSpaceRect::FragmentShader);
    renderer->setUniform(ssRectProg.get(), "texture", 0);
}

GameRenderer::~GameRenderer() {
    glDeleteFramebuffers(1, &framebufferName);
}

void GameRenderer::setupRender() {
    // Set the viewport
    const glm::ivec2& vp = getRenderer().getViewport();
    glViewport(0, 0, vp.x, vp.y);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GameRenderer::renderWorld(GameWorld* world, const ViewCamera& camera,
                               float alpha) {
    const auto& state = world->state;

    _renderAlpha = alpha;
    _renderWorld = world;

    // Store the input camera,
    _camera = camera;

    setupRender();

    glBindVertexArray(vao);

    float tod = world->getHour() + world->getMinute() / 60.f;

    const auto currentWeather = WeatherCondition(state->basic.nextWeather);
    const auto lastWeather = WeatherCondition(state->basic.lastWeather);
    const auto weatherTransition = state->basic.weatherInterpolation;
    auto weather = world->data->weather.interpolate(lastWeather,
                                                    currentWeather,
                                                    weatherTransition,
                                                    tod);

    float theta = (tod / (60.f * 24.f) - 0.5f) * 2.f * glm::pi<float>();
    glm::vec3 sunDirection{
        sin(theta), 0.0, cos(theta),
    };
    sunDirection = glm::normalize(sunDirection);

    _camera.frustum.near = world->state->cameraNear;
    _camera.frustum.far = weather.farClipping;

    auto view = _camera.getView();
    auto proj = _camera.frustum.projection();

    Renderer::SceneUniformData sceneParams{
        proj,
        view,
        glm::vec4{weather.ambientColor, 0.0f},
        glm::vec4{weather.directLightColor, 0.0f},
        glm::vec4{weather.skyBottomColor, 1.f},
        glm::vec4{camera.position, 0.f},
        weather.fogStart,
        _camera.frustum.far};

    renderer->setSceneParameters(sceneParams);

    renderer->clear(glm::vec4(weather.skyBottomColor, 1.f));

    _camera.frustum.update(proj * view);
    if (cullOverride) {
        cullingCamera.frustum.update(cullingCamera.frustum.projection() *
                                     cullingCamera.getView());
    }

    culled = 0;

    renderObjects(world);

    renderer->pushDebugGroup("Water");

    water.render(*this, world);

    profWater = renderer->popDebugGroup();

    renderer->pushDebugGroup("Sky");

    glBindVertexArray(vao);

    Renderer::DrawParameters dp;
    dp.start = 0;
    dp.count = skydomeSegments * skydomeRows * 6;

    renderer->useProgram(skyProg.get());
    renderer->setUniform(skyProg.get(), "TopColor",
                         glm::vec4{weather.skyTopColor, 1.f});
    renderer->setUniform(skyProg.get(), "BottomColor",
                         glm::vec4{weather.skyBottomColor, 1.f});

    renderer->draw(glm::mat4(1.0f), &skyDbuff, dp);

    profSky = renderer->popDebugGroup();

    renderer->pushDebugGroup("Effects");
    renderEffects(world);
    profEffects = renderer->popDebugGroup();

    glDisable(GL_DEPTH_TEST);

    GLuint splashTexName = 0;
    const auto fc = world->state->fadeColour;
    if ((fc.r + fc.g + fc.b) == 0 && !world->state->currentSplash.empty()) {
        auto splash = world->data->findSlotTexture("generic", world->state->currentSplash);
        if (splash) {
            splashTexName = splash->getName();
        }
    }

    if ((world->state->isCinematic || world->state->currentCutscene) &&
        splashTexName != 0 && !world->isPaused()) {
        renderLetterbox();
    }

    if (!world->isPaused()) {
        renderSplash(world, splashTexName, fc);
    }

    if ((world->state->isCinematic || world->state->currentCutscene) &&
        splashTexName == 0 && !world->isPaused()) {
        renderLetterbox();
    }

    renderPostProcess();
}

void GameRenderer::renderObjects(const GameWorld *world) {
    RW_PROFILE_SCOPE(__func__);

    renderer->useProgram(worldProg.get());
    RenderList renderList = createObjectRenderList(world);

    renderer->pushDebugGroup("Objects");
    renderer->pushDebugGroup("RenderList");
    renderer->drawBatched(renderList);

    renderer->popDebugGroup();
    profObjects = renderer->popDebugGroup();
}

RenderList GameRenderer::createObjectRenderList(const GameWorld *world) {
    RW_PROFILE_SCOPE(__func__);
    // This is sequential at the moment, it should be easy to make it
    // run in parallel with a good threading system.
    RenderList renderList;
    // Naive optimisation, assume 50% hitrate
    renderList.reserve(static_cast<size_t>(world->allObjects.size() * 0.5f));

    ObjectRenderer objectRenderer(_renderWorld,
                                  (cullOverride ? cullingCamera : _camera),
                                  _renderAlpha);

    // World Objects
    for (auto object : world->allObjects) {
        objectRenderer.buildRenderList(object, renderList);
    }

    // Area indicators
    auto sphereModel = getSpecialModel(ZoneCylinderA);
    for (auto &i : world->getAreaIndicators()) {
        glm::mat4 m(1.f);
        m = translate(m, i.position);
        m = scale(
                m, glm::vec3(i.radius +
                             0.15f * sin(_renderWorld->getGameTime() * 5.f)));

        objectRenderer.renderClump(sphereModel.get(), m, nullptr, renderList);
    }

    // Render arrows above anything that isn't radar only (or hidden)
    auto arrowModel = getSpecialModel(Arrow);
    for (auto &blip : world->state->radarBlips) {
        auto dm = blip.second.display;
        if (dm == BlipData::Hide || dm == BlipData::RadarOnly) {
            continue;
        }

        glm::mat4 model{1.0f};

        if (blip.second.target > 0) {
            auto object = world->getBlipTarget(blip.second);
            if (object) {
                model = object->getTimeAdjustedTransform(_renderAlpha);
            }
        } else {
            model = translate(model, blip.second.coord);
        }

        float a = world->getGameTime() * glm::pi<float>();
        model = translate(model,
                          glm::vec3(0.f, 0.f, 2.5f + sin(a) * 0.5f));
        model = rotate(model, a, glm::vec3(0.f, 0.f, 1.f));
        model = scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
        objectRenderer.renderClump(arrowModel.get(), model, nullptr, renderList);
    }
    culled += objectRenderer.culled;

    RW_PROFILE_SCOPE("sortRenderList");
    // Also parallelizable
    // Earlier position in the array means earlier object's rendering
    // Transparent objects should be sorted and rendered after opaque
    sort(renderList.begin(), renderList.end(),
         [](const Renderer::RenderInstruction &a,
            const Renderer::RenderInstruction &b) {
             if (a.drawInfo.blendMode == BlendMode::BLEND_NONE && b.drawInfo.blendMode != BlendMode::BLEND_NONE)
                 return true;
             if (a.drawInfo.blendMode != BlendMode::BLEND_NONE && b.drawInfo.blendMode == BlendMode::BLEND_NONE)
                 return false;
             return (a.sortKey > b.sortKey);
         });

    return renderList;
}

void GameRenderer::renderSplash(GameWorld* world, GLuint splashTexName, glm::u16vec3 fc) {
    float fadeTimer = world->getGameTime() - world->state->fadeStart;

    if (splashTexName != 0) {
        fc = glm::u16vec3(0, 0, 0);
    }

    float fadeFrac = 1.f;
    if (world->state->fadeTime > 0.f) {
        fadeFrac = std::min(fadeTimer / world->state->fadeTime, 1.f);
    }

    float a = world->state->fadeIn ? 1.f - fadeFrac : fadeFrac;
    if (a <= 0.f) {
        return;
    }

    glm::vec4 fadeNormed(fc.r / 255.f, fc.g / 255.f, fc.b / 255.f, a);

    renderer->useProgram(ssRectProg.get());
    renderer->setUniform(ssRectProg.get(), "colour", fadeNormed);
    renderer->setUniform(ssRectProg.get(), "size", glm::vec2{1.f, 1.f});
    renderer->setUniform(ssRectProg.get(), "offset", glm::vec2{0.f, 0.f});

    Renderer::DrawParameters wdp;
    wdp.depthMode = DepthMode::OFF;
    wdp.blendMode = BlendMode::BLEND_ALPHA;
    wdp.count = ssRectGeom.getCount();
    wdp.textures = {{splashTexName}};

    renderer->drawArrays(glm::mat4(1.0f), &ssRectDraw, wdp);
}

void GameRenderer::renderPostProcess() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glStencilMask(0xFF);
    glClearStencil(0x00);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    renderer->useProgram(postProg.get());

    Renderer::DrawParameters wdp;
    wdp.start = 0;
    wdp.count = ssRectGeom.getCount();
    wdp.textures = {{fbTextures[0]}};
    wdp.depthMode = DepthMode::OFF;

    renderer->drawArrays(glm::mat4(1.0f), &ssRectDraw, wdp);
}

void GameRenderer::renderEffects(GameWorld* world) {
    renderer->useProgram(particleProg.get());

    auto cpos = _camera.position;
    auto cfwd = glm::normalize(glm::inverse(_camera.rotation) *
                               glm::vec3(0.f, 1.f, 0.f));

    auto& effects = world->effects;

    std::sort(effects.begin(), effects.end(),
              [&](const auto& a, const auto& b) {
                  return glm::distance(a->position, cpos) >
                         glm::distance(b->position, cpos);
              });

    for (auto& fx : effects) {
        // Other effects not implemented yet
        if (fx->getType() != Particle) continue;
        auto particle = static_cast<ParticleFX*>(fx.get());

        auto& p = particle->position;

        // Figure the direction to the camera center.
        auto amp = cpos - p;
        glm::vec3 ptc = particle->up;

        if (particle->orientation == ParticleFX::UpCamera) {
            ptc = glm::normalize(amp - (glm::dot(amp, cfwd)) * cfwd);
        } else if (particle->orientation == ParticleFX::Camera) {
            ptc = amp;
        }

        ptc = glm::normalize(ptc);

        glm::mat4 transformMat(1.f);

        glm::mat4 lookMat = glm::lookAt(
            glm::vec3(0.0f,0.0f,0.0f),
            ptc,
            glm::vec3(0.0f,0.0f,1.0f));

        transformMat = glm::scale(glm::translate(transformMat,p),
            glm::vec3(particle->size,1.0f)) * glm::inverse(lookMat);

        Renderer::DrawParameters dp;
        dp.textures = {{particle->texture->getName()}};
        dp.ambient = 1.f;
        dp.colour = glm::u8vec4(particle->colour * 255.f);
        dp.start = 0;
        dp.count = 4;
        dp.blendMode = BlendMode::BLEND_ADDITIVE;
        dp.diffuse = 1.f;

        renderer->drawArrays(transformMat, &particleDraw, dp);
    }
}

void GameRenderer::drawTexture(TextureData* texture, glm::vec4 extents) {
    drawRect({0.f, 0.f, 0.f, 1.f}, texture, extents);
}

void GameRenderer::drawColour(const glm::vec4& colour, glm::vec4 extents) {
    drawRect(colour, nullptr, extents);
}

void GameRenderer::drawRect(const glm::vec4& colour, TextureData* texture, glm::vec4& extents) {
    // Move into NDC
    extents.x /= renderer->getViewport().x;
    extents.y /= renderer->getViewport().y;
    extents.z /= renderer->getViewport().x;
    extents.w /= renderer->getViewport().y;
    extents.x += extents.z / 2.f;
    extents.y += extents.w / 2.f;
    extents.x -= .5f;
    extents.y -= .5f;
    extents *= glm::vec4(2.f, -2.f, 1.f, 1.f);

    renderer->useProgram(ssRectProg.get());
    renderer->setUniform(ssRectProg.get(), "colour", colour);
    renderer->setUniform(ssRectProg.get(), "size", glm::vec2{extents.z, extents.w});
    renderer->setUniform(ssRectProg.get(), "offset", glm::vec2{extents.x, extents.y});

    Renderer::DrawParameters wdp;
    wdp.depthMode = DepthMode::OFF;
    wdp.blendMode = BlendMode::BLEND_ALPHA;
    wdp.count = ssRectGeom.getCount();
    wdp.textures = {{texture ? texture->getName() : 0}};

    renderer->drawArrays(glm::mat4(1.0f), &ssRectDraw, wdp);
}

void GameRenderer::renderLetterbox() {
    constexpr float cinematicExperienceSize = 0.15f;
    renderer->useProgram(ssRectProg.get());
    renderer->setUniform(ssRectProg.get(), "colour", glm::vec4{0.f, 0.f, 0.f, 1.f});
    renderer->setUniform(ssRectProg.get(), "size", glm::vec2{1.f, cinematicExperienceSize});
    renderer->setUniform(ssRectProg.get(), "offset", glm::vec2{0.f,-1.f * (1.f - cinematicExperienceSize)});
    Renderer::DrawParameters wdp;
    wdp.depthMode = DepthMode::OFF;
    wdp.blendMode = BlendMode::BLEND_NONE;
    wdp.count = ssRectGeom.getCount();
    wdp.textures = {{0}};

    renderer->drawArrays(glm::mat4(1.0f), &ssRectDraw, wdp);
    renderer->setUniform(ssRectProg.get(), "offset", glm::vec2{0.f, 1.f * (1.f - cinematicExperienceSize)});
    renderer->drawArrays(glm::mat4(1.0f), &ssRectDraw, wdp);
}

void GameRenderer::setViewport(int w, int h) {
    auto& lastViewport = renderer->getViewport();
    if (lastViewport.x != w || lastViewport.y != h) {
        renderer->setViewport({w, h});

        glBindTexture(GL_TEXTURE_2D, fbTextures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, fbTextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, w, h, 0, GL_RED, GL_FLOAT,
                     nullptr);

        glBindRenderbuffer(GL_RENDERBUFFER, fbRenderBuffers[0]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    }
}
