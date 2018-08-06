#include "ViewerWidget.hpp"
#include <QFileDialog>
#include <QMouseEvent>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <render/GameRenderer.hpp>
#include <render/ObjectRenderer.hpp>
#include <render/TextRenderer.hpp>

constexpr float kViewFov = glm::radians(90.0f);

namespace {
ViewCamera OrbitCamera (const glm::vec2& viewPort, const glm::vec2& viewAngles,
                        float viewDistance, glm::mat4& view, glm::mat4& proj)
{
    ViewCamera vc;
    glm::vec3 eye(sin(viewAngles.x) * cos(viewAngles.y),
                  cos(viewAngles.x) * cos(viewAngles.y), sin(viewAngles.y));

    vc.position = eye * viewDistance;
    vc.frustum.aspectRatio = viewPort.x / viewPort.y;
    proj = vc.frustum.projection();
    view = glm::lookAt(vc.position, {0.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    vc.rotation = -glm::quat_cast(view);
    vc.frustum.update(proj * view);
    return vc;
}
}

ViewerWidget::ViewerWidget(QOpenGLContext* context, QWindow* parent)
    : QWindow(parent)
    , context(context) {
    setSurfaceType(OpenGLSurface);
}

struct WidgetVertex {
    float x, y, z;
    static const AttributeList vertex_attributes() {
        return {{ATRS_Position, 3, sizeof(WidgetVertex), 0ul}};
    }
};

std::vector<WidgetVertex> widgetVerts = {{-.5f, 0.f, 0.f}, {.5f, 0.f, 0.f},
                                         {0.f, -.5f, 0.f}, {0.f, .5f, 0.f},
                                         {0.f, 0.f, -.5f}, {0.f, 0.f, .5f}};

void ViewerWidget::initGL() {
    _frameWidgetDraw = new DrawBuffer;
    _frameWidgetDraw->setFaceType(GL_LINES);
    _frameWidgetGeom = new GeometryBuffer;
    _frameWidgetGeom->uploadVertices(widgetVerts);
    _frameWidgetDraw->addGeometry(_frameWidgetGeom);

    glGenTextures(1, &whiteTex);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    GLuint tex = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 &tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ViewerWidget::drawModel(GameRenderer& r, ClumpPtr& model) {
    glm::mat4 view{1.0f}, proj{1.0f};
    const auto& vc = OrbitCamera({width(), height()},
                                 viewAngles,
                                 viewDistance,
                                 view, proj);

    r.getRenderer()->useProgram(r.worldProg.get());
    r.getRenderer()->setSceneParameters(
            {proj, view, glm::vec4(0.15f), glm::vec4(0.7f), glm::vec4(1.f),
             glm::vec4(0.f), 90.f, vc.frustum.far});
    model->getFrame()->updateHierarchyTransform();

    ObjectRenderer _renderer(world(), vc, 1.f, 0);
    RenderList renders;
    _renderer.renderClump(model.get(), glm::mat4(1.0f), nullptr, renders);
    r.getRenderer()->drawBatched(renders);

    drawFrameWidget(model->getFrame().get());
    r.renderPostProcess();
}

void ViewerWidget::drawObject(GameRenderer &r, GameObject *object) {
    glm::mat4 view{1.0f}, proj{1.0f};
    const auto& vc = OrbitCamera({width(), height()},
                                 viewAngles,
                                 viewDistance,
                                 view, proj);

    r.getRenderer()->useProgram(r.worldProg.get());
    r.getRenderer()->setSceneParameters(
            {proj, view, glm::vec4(0.15f), glm::vec4(0.7f), glm::vec4(1.f),
             glm::vec4(0.f), 90.f, vc.frustum.far});

    ObjectRenderer objectRenderer(world(), vc, 1.f, 0);
    RenderList renders;
    objectRenderer.buildRenderList(object, renders);
    std::sort(renders.begin(), renders.end(),
              [](const Renderer::RenderInstruction& a,
                 const Renderer::RenderInstruction& b) {
                  return a.sortKey < b.sortKey;
              });
    r.getRenderer()->drawBatched(renders);
    r.renderPostProcess();
}

void ViewerWidget::drawWorld(GameRenderer& r) {
    ViewCamera vc;
    vc.frustum.fov = kViewFov;
    vc.frustum.far = 1000.f;
    vc.frustum.near = 0.1f;
    vc.position = viewPosition;
    vc.rotation = glm::angleAxis(glm::half_pi<float>() + viewAngles.x,
                                 glm::vec3(0.f, 0.f, 1.f)) *
                  glm::angleAxis(viewAngles.y, glm::vec3(0.f, 1.f, 0.f));
    vc.frustum.aspectRatio = width() / (height() * 1.f);
    r.renderWorld(world(), vc, 0.f);
}

void ViewerWidget::drawText(GameRenderer& r) {
    for(auto &textInfo : textInfos) {
        _renderer->text.renderText(textInfo, false);
    }
    r.renderPostProcess();
}

void ViewerWidget::paintGL() {
    glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
    glClearColor(0.3f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (world() == nullptr) return;

    RW_CHECK(_renderer != nullptr, "GameRenderer is null");
    auto& r = *_renderer;
    r.getRenderer()->invalidate();
    r.setViewport(width() * devicePixelRatio(), height() * devicePixelRatio());

    glEnable(GL_DEPTH_TEST);

    r.getRenderer()->invalidate();
    r.setupRender();

    switch (_viewMode) {
    case Mode::Model:
        if (_model) drawModel(r, _model);
        break;
    case Mode::Object:
        if (_object) drawObject(r, _object);
        break;
    case Mode::World:
        drawWorld(r);
        break;
    case Mode::Text:
        drawText(r);
        break;
    }
}

void ViewerWidget::drawFrameWidget(ModelFrame* f, const glm::mat4& m) {
    auto thisM = m * f->getTransform();
    Renderer::DrawParameters dp;
    dp.count = _frameWidgetGeom->getCount();
    dp.start = 0;
    dp.ambient = 1.f;
    dp.diffuse = 1.f;
    if (f == selectedFrame) {
        dp.colour = {255, 255, 0, 255};
        // Sorry!
        glLineWidth(10.f);
    } else {
        dp.colour = {255, 255, 255, 255};
        glLineWidth(1.f);
    }
    dp.textures = {whiteTex};

    RW_CHECK(_renderer != nullptr, "GameRenderer is null");
    if(_renderer != nullptr) {
        _renderer->getRenderer()->drawArrays(thisM, _frameWidgetDraw, dp);
    }

    for (auto c : f->getChildren()) {
        drawFrameWidget(c.get(), thisM);
    }
}

GameWorld* ViewerWidget::world() {
    return _world;
}

void ViewerWidget::showObject(quint16 item) {
    RW_ASSERT(world());
    _viewMode = Mode::Object;
    _objectID = item;

    if (_object) _world->destroyObject(_object);
    _object = nullptr;

    auto def = world()->data->modelinfo[item].get();
    if (!def) {
        return;
    }

    if (!world()->data->loadModel(item)) {
        return;
    }

    switch (def->type()) {
    default:
        _object = _world->createInstance(item, {});
        break;
    case ModelDataType::PedInfo:
        _object = _world->createPedestrian(item, {});
        break;
    case ModelDataType::VehicleInfo:
        _object = _world->createVehicle(item, {});
        break;
    }

    RW_CHECK(_object != nullptr, "Dummy Object is null");

    if (_object->getModel()) {
        auto objectRadius = _object->getModel()->getBoundingRadius();
        viewDistance = objectRadius * 2;
        viewAngles.x = glm::radians(-45.f);
        viewAngles.y = glm::radians(22.5f);
    }
}

void ViewerWidget::clearText() {
    textInfos.clear();
}

void ViewerWidget::showText(const TextRenderer::TextInfo &ti) {
    textInfos.push_back(ti);
}

void ViewerWidget::showModel(ClumpPtr model) {
    _viewMode = Mode::Model;
    _model = model;
    textInfos.clear();
}

void ViewerWidget::selectFrame(ModelFrame* frame) {
    selectedFrame = frame;
}

void ViewerWidget::exportModel() {
#if 0
    QString toSv = QFileDialog::getSaveFileName(
        this, "Export Model", QDir::homePath(), "Model (*.DFF)");

    if (toSv.size() == 0) return;

	auto it = world()->objectTypes.find(currentObjectID);
	if( it != world()->objectTypes.end() ) {
		for( auto& archive : world()->data.archives ) {
			for(size_t i = 0; i < archive.second.getAssetCount(); ++i) {
				auto& assetI = archive.second.getAssetInfoByIndex(i);;
				std::string q(assetI.name);
				std::transform(q.begin(), q.end(), q.begin(), ::tolower);
				if( q.find(it->second->modelName) != q.npos ) {
					archive.second.saveAsset(q, toSv.toStdString());
				}
			}
		}
	}
#endif
}

void ViewerWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Shift) moveFast = true;

    glm::vec3 movement{};
    if (e->key() == Qt::Key_W) movement.y += moveFast ? 10.f : 1.f;
    if (e->key() == Qt::Key_S) movement.y -= moveFast ? 10.f : 1.f;
    if (e->key() == Qt::Key_A) movement.x -= moveFast ? 10.f : 1.f;
    if (e->key() == Qt::Key_D) movement.x += moveFast ? 10.f : 1.f;

    if (movement.length() > 0.f) {
        movement = (glm::angleAxis(viewAngles.x, glm::vec3(0.f, 0.f, 1.f)) *
                    glm::angleAxis(viewAngles.y, glm::vec3(-1.f, 0.f, 0.f))) *
                   movement;
        viewPosition += movement;
    }
}

void ViewerWidget::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Shift) moveFast = false;
}

ClumpPtr ViewerWidget::currentModel() const {
    return _model;
}

GameObject* ViewerWidget::currentObject() const {
    return _object;
}

void ViewerWidget::mousePressEvent(QMouseEvent* e) {
    dragging = true;
    dstart = e->localPos();
    dastart = viewAngles;
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent*) {
    dragging = false;
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* e) {
    if (dragging) {
        auto d = e->localPos() - dstart;
        viewAngles = dastart + glm::vec2(d.x(), d.y()) * 0.01f;
    }
}

void ViewerWidget::wheelEvent(QWheelEvent* e) {
    viewDistance = qMax(viewDistance - e->angleDelta().y() / 240.f, 0.5f);
}

void ViewerWidget::gameLoaded(GameWorld *world, GameRenderer *renderer) {
    _world = world;
    _renderer = renderer;
}

void ViewerWidget::renderNow() {
    if (!isExposed()) {
        return;
    }

    context->makeCurrent(this);

    if (!initialised) {
        initGL();
        initialised = true;
    }

    paintGL();
    context->swapBuffers(this);

    requestUpdate();
}

bool ViewerWidget::event(QEvent *e) {
    switch(e->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default: return QWindow::event(e);
    }
}

void ViewerWidget::exposeEvent(QExposeEvent *) {
    if (isExposed()) {
        requestUpdate();
    }
}

