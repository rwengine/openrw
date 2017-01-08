#include "ViewerWidget.hpp"
#include <QFileDialog>
#include <QMouseEvent>
#include <algorithm>
#include <data/Clump.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/GameObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <render/GameRenderer.hpp>
#include <render/ObjectRenderer.hpp>
#include <render/OpenGLRenderer.hpp>

ViewerWidget::ViewerWidget(QGLFormat g, QWidget* parent,
                           const QGLWidget* shareWidget, Qt::WindowFlags f)
    : QGLWidget(g, parent, shareWidget, f)
    , gworld(nullptr)
    , activeModel(nullptr)
    , selectedFrame(nullptr)
    , dummyObject(nullptr)
    , currentObjectID(0)
    , _lastModel(nullptr)
    , canimation(nullptr)
    , viewDistance(1.f)
    , dragging(false)
    , moveFast(false)
    , _frameWidgetDraw(nullptr)
    , _frameWidgetGeom(nullptr) {
    setFocusPolicy(Qt::StrongFocus);
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

void ViewerWidget::initializeGL() {
    QGLWidget::initializeGL();
    timer.setInterval(25);
    connect(&timer, SIGNAL(timeout()), SLOT(updateGL()));
    timer.start();

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

void ViewerWidget::resizeGL(int w, int h) {
    QGLWidget::resizeGL(w, h);
    glViewport(0, 0, w, h);
}

void ViewerWidget::paintGL() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width(), height());

    if (world() == nullptr) return;

    auto& r = *renderer;

    r.setViewport(width(), height());

    if (dummyObject && dummyObject->animator && dummyObject->skeleton) {
        dummyObject->animator->tick(1.f / 60.f);
        dummyObject->skeleton->interpolate(1.f);
    }

    r.getRenderer()->invalidate();

    glEnable(GL_DEPTH_TEST);

    glm::mat4 m(1.f);

    r.getRenderer()->useProgram(r.worldProg);

    ViewCamera vc;

    float viewFov = glm::radians(45.f);

    vc.frustum.far = 500.f;
    vc.frustum.near = 0.1f;
    vc.frustum.fov = viewFov;
    vc.frustum.aspectRatio = width() / (height() * 1.f);

    Clump* model = activeModel;
    if (model != _lastModel) {
        _lastModel = model;
        emit modelChanged(_lastModel);
    }

    glm::vec3 eye(sin(viewAngles.x) * cos(viewAngles.y),
                  cos(viewAngles.x) * cos(viewAngles.y), sin(viewAngles.y));

    if (model) {
        model->getFrame()->updateHierarchyTransform();

        // Ensure camera is still accurate
        vc.position = eye * viewDistance;
        glm::mat4 proj = vc.frustum.projection();
        glm::mat4 view = glm::lookAt(vc.position, glm::vec3(0.f, 0.f, 0.f),
                                     glm::vec3(0.f, 0.f, 1.f));
        vc.rotation = -glm::quat_cast(view);
        vc.frustum.update(proj * view);

        r.getRenderer()->setSceneParameters(
            {proj, view, glm::vec4(0.15f), glm::vec4(0.7f), glm::vec4(1.f),
             glm::vec4(0.f), 90.f, vc.frustum.far});

        r.getRenderer()->invalidate();

        r.setupRender();

        ObjectRenderer renderer(world(), vc, 1.f, 0);
        RenderList renders;
        renderer.renderClump(model, glm::mat4(), nullptr, renders);
        r.getRenderer()->drawBatched(renders);

        drawFrameWidget(model->getFrame().get());
        r.renderPostProcess();
    } else if (world()->allObjects.size() > 0) {
        vc.frustum.fov = glm::radians(90.f);
        vc.frustum.far = 1000.f;
        vc.position = viewPosition;
        vc.rotation = glm::angleAxis(glm::half_pi<float>() + viewAngles.x,
                                     glm::vec3(0.f, 0.f, 1.f)) *
                      glm::angleAxis(viewAngles.y, glm::vec3(0.f, 1.f, 0.f));
        r.renderWorld(world(), vc, 0.f);
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
    renderer->getRenderer()->drawArrays(thisM, _frameWidgetDraw, dp);

    for (auto c : f->getChildren()) {
        drawFrameWidget(c.get(), thisM);
    }
}

GameWorld* ViewerWidget::world() {
    return gworld;
}

void ViewerWidget::showObject(qint16 item) {
    currentObjectID = item;

    if (dummyObject) gworld->destroyObject(dummyObject);

    auto def = world()->data->modelinfo[item].get();

    if (def) {
        switch (def->type()) {
            default:
                dummyObject = gworld->createInstance(item, {});
                break;
            case ModelDataType::PedInfo:
                dummyObject = gworld->createPedestrian(item, {});
                break;
            case ModelDataType::VehicleInfo:
                dummyObject = gworld->createVehicle(item, {});
                break;
        }

        RW_CHECK(dummyObject != nullptr, "Dummy Object is null");
        if (dummyObject != nullptr) {
            activeModel = dummyObject->getModel();
        }
    }
}

void ViewerWidget::showModel(Clump* model) {
    if (dummyObject) gworld->destroyObject(dummyObject);
    dummyObject = nullptr;
    activeModel = model;
}

void ViewerWidget::selectFrame(ModelFrame* frame) {
    selectedFrame = frame;
}

void ViewerWidget::exportModel() {
    QString toSv = QFileDialog::getSaveFileName(
        this, "Export Model", QDir::homePath(), "Model (*.DFF)");

    if (toSv.size() == 0) return;

#if 0
	auto it = world()->objectTypes.find(currentObjectID);
	if( it != world()->objectTypes.end() ) {
		for( auto& archive : world()->data.archives ) {
			for(size_t i = 0; i < archive.second.getAssetCount(); ++i) {
				auto& assetI = archive.second.getAssetInfoByIndex(i);
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

void ViewerWidget::dataLoaded(GameWorld* world) {
    gworld = world;
}

void ViewerWidget::setRenderer(GameRenderer* render) {
    renderer = render;
}

void ViewerWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Shift) moveFast = true;

    glm::vec3 movement;
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

Clump* ViewerWidget::currentModel() const {
    return activeModel;
}

GameObject* ViewerWidget::currentObject() const {
    return dummyObject;
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
