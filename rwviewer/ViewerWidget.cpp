#include "ViewerWidget.hpp"
#include <render/Model.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include <engine/GameObject.hpp>
#include <engine/Animator.hpp>

ViewerWidget::ViewerWidget(QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f)
: QGLWidget(parent, shareWidget, f), gworld(nullptr), dummyObject(nullptr),
  cmodel(nullptr), canimation(nullptr), viewDistance(1.f), dragging(false)
{
}

void ViewerWidget::initializeGL()
{
	QGLWidget::initializeGL();
	timer.setInterval(16);
	connect(&timer, SIGNAL(timeout()), SLOT(updateGL()));
	timer.start();
	
	glewExperimental = 1;
	glewInit();
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

	auto& r = gworld->renderer;
	
	r.camera.frustum.far = 100.f;
	r.camera.frustum.near = 0.1f;
	r.camera.frustum.fov = 90.f;
	r.camera.frustum.aspectRatio = width()/(height()*1.f);

	if(dummyObject && dummyObject->animator) {
		dummyObject->animator->tick(1.f/60.f);
	}
	
	if(cmodel) {
		glEnable(GL_DEPTH_TEST);
		
		glm::mat4 m;
		
		glUseProgram(r.worldProgram);

		glUniform1f(r.uniFogStart, 90.f);
		glUniform1f(r.uniFogEnd, 100.f);

		glUniform4f(r.uniAmbientCol, .1f, .1f, .1f, 1.f);
		glUniform4f(r.uniDynamicCol, 1.f, 1.f, 1.f, 1.f);
		//glUniform3f(uniSunDirection, sunDirection.x, sunDirection.y, sunDirection.z);
		glUniform1f(r.uniMatDiffuse, 0.9f);
		glUniform1f(r.uniMatAmbient, 0.1f);
		
		glm::mat4 proj = r.camera.frustum.projection();
		glm::vec3 eye(sin(viewAngles.x) * cos(viewAngles.y), cos(viewAngles.x) * cos(viewAngles.y), sin(viewAngles.y));
		glm::mat4 view = glm::lookAt(eye * viewDistance, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		glUniformMatrix4fv(r.uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(r.uniProj, 1, GL_FALSE, glm::value_ptr(proj));
		
		gworld->renderer.renderModel(cmodel->model, m, dummyObject);
	}
}

GameWorld* ViewerWidget::world()
{
	return gworld;
}

void ViewerWidget::showItem(qint16 item)
{
	// TODO: actually show items.
}

void ViewerWidget::showAnimation(Animation *anim)
{
	canimation = anim;
	if(dummyObject) {
		if(dummyObject->animator == nullptr) {
			dummyObject->animator = new Animator;
			dummyObject->animator->setModel(dummyObject->model->model);
		}
		dummyObject->animator->setAnimation(canimation);
	}
}

ModelHandle* ViewerWidget::currentModel() const
{
	return cmodel;
}

void ViewerWidget::setGamePath(const std::string &path)
{
	if( gworld ) delete gworld;
	gworld = new GameWorld(path);
	gworld->gameData.load();

	for(auto it = gworld->gameData.ideLocations.begin();
		it != gworld->gameData.ideLocations.end();
		++it) {
		gworld->defineItems(it->second);
	}

	emit dataLoaded(gworld);
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

