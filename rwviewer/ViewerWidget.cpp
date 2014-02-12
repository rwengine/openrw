#include "ViewerWidget.hpp"
#include <render/Model.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>

ViewerWidget::ViewerWidget(QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f)
: QGLWidget(parent, shareWidget, f), gworld(nullptr), cmodel(nullptr), 
 viewDistance(1.f), dragging(false), fm(ViewerWidget::UNK)
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
	
	gworld = new GameWorld("");
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
	
	auto& r = gworld->renderer;
	
	r.camera.frustum.far = 100.f;
	r.camera.frustum.near = 0.1f;
	r.camera.frustum.fov = 60.f;
	r.camera.frustum.aspectRatio = width()/(height()*1.f);
	
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
		
		gworld->renderer.renderModel(cmodel, m);
	}
}

GameWorld* ViewerWidget::world()
{
	return gworld;
}

void ViewerWidget::showFile(const QString& file)
{
	cmodel = nullptr;
	currentFile = file;
	QString low = file.toLower();
	if(low.endsWith("dff")) {
		showDFF(file);
	}
	else if(low.endsWith("txd")) {
		showTXD(file);
	}
	emit fileOpened(file);
}

void ViewerWidget::showDFF(const QString& file)
{
	gworld->gameData.loadDFF(file.toStdString());
	QString basename(file.left(file.size()-4));
	// HACK this
	gworld->gameData.loadTXD((basename+".txd").toStdString());
	auto mit = gworld->gameData.models.find(basename.toStdString());
	if(mit != gworld->gameData.models.end()) {
		// TODO better error handling
		cmodel = mit->second;
		float radius = 0.f;
		for(auto& g 
			: cmodel->geometries) {
			radius = std::max(
				radius,
				glm::length(g->geometryBounds.center)+g->geometryBounds.radius);
		}
		radius *= 4.f;
		viewDistance = (radius/2.f) / tan(gworld->renderer.camera.frustum.aspectRatio/2.f);
	}
	fm = ViewerWidget::DFF;
}

void ViewerWidget::showTXD(const QString& file)
{
	fm = ViewerWidget::TXD;
}

Model* ViewerWidget::currentModel() const
{
	return cmodel;
}

ViewerWidget::FileMode ViewerWidget::fileMode() const
{
	return fm;
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
	viewDistance = qMax(viewDistance - e->angleDelta().y() / 240.f, 1.f);
}

