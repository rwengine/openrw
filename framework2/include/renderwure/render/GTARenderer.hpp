#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <memory>

#include <renderwure/render/ViewCamera.hpp>

class Model;
class GTAEngine;
class GTARenderer
{
public:
	
	GTARenderer();
	
	ViewCamera camera;
	
	/// The numer of things rendered by the last renderWorld
	size_t rendered;
	size_t culled;

	GLint uniModel, uniProj, uniView, uniCol, uniAmbientCol, uniSunDirection, uniDynamicCol;
	GLint uniMatDiffuse, uniMatAmbient, uniFogStart, uniFogEnd;
	GLint posAttrib, normalAttrib, texAttrib, colourAttrib;
	GLuint worldProgram;
	GLuint skyProgram;
	GLint skyUniView, skyUniProj, skyUniTop, skyUniBottom;
	
	GLuint planeVBO, skydomeVBO;
	
	void renderWorld(GTAEngine* engine);
	
	void renderNamedFrame(GTAEngine* engine, const std::unique_ptr<Model>&, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const std::string& name);
	
	void renderObject(GTAEngine* engine, const std::unique_ptr<Model>&, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
};

#endif