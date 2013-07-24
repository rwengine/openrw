#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <memory>

#include <renderwure/render/ViewCamera.hpp>

class Model;
class GTAEngine;
class GTAObject;

class GTARenderer
{
    GTAEngine* engine;

public:
	
    GTARenderer(GTAEngine*);
	
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
	
	/// Internal VAO to avoid clobbering global state.
	GLuint vao;
	
	GLuint planeVBO, skydomeVBO;
	
    void renderWorld();
	
    void renderNamedFrame(Model*, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const std::string& name);

    void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, GTAObject* = nullptr);

    void renderModel(Model*, const glm::mat4& modelMatrix, GTAObject* = nullptr);
};

#endif
