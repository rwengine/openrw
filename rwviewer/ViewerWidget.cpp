#include "ViewerWidget.hpp"
#include <render/Model.hpp>
#include <render/GameRenderer.hpp>
#include <render/OpenGLRenderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include <engine/GameObject.hpp>
#include <engine/Animator.hpp>
#include <QFileDialog>
#include <algorithm>

#include <objects/InstanceObject.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>


ViewerWidget::ViewerWidget(QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f)
: QGLWidget(parent, shareWidget, f), gworld(nullptr), activeModel(nullptr),
  selectedFrame(nullptr), dummyObject(nullptr), currentObjectID(0),
  _lastModel(nullptr), canimation(nullptr), viewDistance(1.f), dragging(false),
  _frameWidgetDraw(nullptr), _frameWidgetGeom(nullptr)
{
}

struct WidgetVertex {
	float x, y, z;
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 3, sizeof(WidgetVertex),  0ul}
		};
	}
};

std::vector<WidgetVertex> widgetVerts = {
	{-.5f, 0.f, 0.f},
	{ .5f, 0.f, 0.f},
	{ 0.f,-.5f, 0.f},
	{ 0.f, .5f, 0.f},
	{ 0.f, 0.f,-.5f},
	{ 0.f, 0.f, .5f}
};

void ViewerWidget::initializeGL()
{
	QGLWidget::initializeGL();
	timer.setInterval(16);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ViewerWidget::resizeGL(int w, int h)
{
	QGLWidget::resizeGL(w, h);
	glViewport(0, 0, w, h);
}

void ViewerWidget::paintGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, width(), height());
	
	if( gworld == nullptr ) return;

	auto& r = *renderer;

	r.setViewport(width(), height());

	if(dummyObject && dummyObject->animator) {
		dummyObject->animator->tick(1.f/60.f);
	}
	
	if(activeModel) {
		gworld->_work->update();

		r.getRenderer()->invalidate();

		Model* model = activeModel;

		if( model != _lastModel ) {
			_lastModel = model;
			emit modelChanged(_lastModel);
		}

		glEnable(GL_DEPTH_TEST);
		
		glm::mat4 m(1.f);

		r.getRenderer()->useProgram(r.worldProg);

		ViewCamera vc;

		float viewFov = glm::radians(45.f);

		vc.frustum.far = 500.f;
		vc.frustum.near = 0.1f;
		vc.frustum.fov = viewFov;
		vc.frustum.aspectRatio = width()/(height()*1.f);

		glm::mat4 proj = vc.frustum.projection();
		glm::vec3 eye(sin(viewAngles.x) * cos(viewAngles.y), cos(viewAngles.x) * cos(viewAngles.y), sin(viewAngles.y));
		glm::mat4 view = glm::lookAt(eye * viewDistance, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));

		r.getRenderer()->setSceneParameters({ proj, view, glm::vec4(0.15f), glm::vec4(0.7f), glm::vec4(1.f), glm::vec4(0.f), 90.f, vc.frustum.far });

		r.getRenderer()->invalidate();

		r.setupRender();
		if( model )
		{
			r.renderModel(model, m, dummyObject);

			drawFrameWidget(model->frames[model->rootFrameIdx]);
		}
		r.renderPostProcess();
	}
}

void ViewerWidget::drawFrameWidget(ModelFrame* f, const glm::mat4& m)
{
	auto thisM = m * f->getTransform();
	if(f->getGeometries().size() == 0)
	{
		Renderer::DrawParameters dp;
		dp.count = _frameWidgetGeom->getCount();
		dp.start = 0;
		dp.ambient = 1.f;
		dp.diffuse = 1.f;
		if( f == selectedFrame )
		{
			dp.colour = {255, 255, 0, 255};
			// Sorry!
			glLineWidth(10.f);
		}
		else
		{
			dp.colour = {255, 255, 255, 255};
			glLineWidth(1.f);
		}
		dp.textures = { whiteTex };
		renderer->getRenderer()->drawArrays(thisM, _frameWidgetDraw, dp);
	}
	
	for(auto c : f->getChildren()) {
		drawFrameWidget(c, thisM);
	}
}

GameWorld* ViewerWidget::world()
{
	return gworld;
}

void ViewerWidget::showItem(qint16 item)
{
	currentObjectID = item;

	if( dummyObject ) gworld->destroyObject( dummyObject );

	auto def = world()->objectTypes[item];

	if( def )
	{
		if(def->class_type == ObjectData::class_id)
		{
			dummyObject = gworld->createInstance(item, {});
		}
		else if(def->class_type == CharacterData::class_id)
		{
			dummyObject = gworld->createPedestrian(item, {});
		}
		else if(def->class_type == VehicleData::class_id)
		{
			dummyObject = gworld->createVehicle(item, {});
		}
		activeModel = dummyObject->model->resource;
	}
}

void ViewerWidget::showModel(Model* model)
{
	if( dummyObject ) gworld->destroyObject( dummyObject );
	dummyObject = nullptr;
	activeModel = model;
}

void ViewerWidget::selectFrame(ModelFrame* frame)
{
	selectedFrame = frame;
}

void ViewerWidget::exportModel()
{
	QString toSv = QFileDialog::getSaveFileName(this,
												"Export Model",
												QDir::homePath(),
												"Model (*.DFF)");

	if( toSv.size() == 0 ) return;

#if 0
	auto it = world()->objectTypes.find(currentObjectID);
	if( it != world()->objectTypes.end() ) {
		for( auto& archive : world()->gameData.archives ) {
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

void ViewerWidget::dataLoaded(GameWorld *world)
{
	gworld = world;
}

void ViewerWidget::setRenderer(GameRenderer *render)
{
	renderer = render;
}

Model* ViewerWidget::currentModel() const
{
	return activeModel;
}

void ViewerWidget::mousePressEvent(QMouseEvent* e)
{
	dragging = true;
	dstart = e->localPos();
	dastart = viewAngles;
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent*)
{
	dragging = false;
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
    if(dragging) {
		auto d = e->localPos() - dstart;
		viewAngles = dastart + glm::vec2(d.x(), d.y()) * 0.01f;
	}
}

void ViewerWidget::wheelEvent(QWheelEvent* e)
{
	viewDistance = qMax(viewDistance - e->angleDelta().y() / 240.f, 0.5f);
}

