#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <memory>
#include <vector>

#include <render/ViewCamera.hpp>

class Model;
class GameWorld;
class GTAObject;
class Animator;

class GTARenderer
{
    GameWorld* engine;
	
	struct RQueueEntry {
		Model* model;
		size_t g;
		size_t sg;
		glm::mat4 matrix;
		GTAObject* object;
	};
	
	// Internal method for processing sub-geometry
	bool renderSubgeometry(Model* model, size_t g, size_t sg, const glm::mat4& matrix, GTAObject* object, bool queueTransparent = true);
	
	/// Queue of sub-geometry to post-render 
	/// With a faster occulusion culling stage
	/// This could be replaced with a 2nd draw pass.
	std::vector<RQueueEntry> transparentDrawQueue;

public:
	
    GTARenderer(GameWorld*);
	
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
    GLuint vao, debugVAO;
	
    GLuint planeVBO, skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;
	
    /**
     * @brief renderWorld renders the world.
     */
    void renderWorld();

	void renderNamedFrame(Model*, const glm::mat4& matrix, const std::string& name);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, GTAObject* = nullptr);

	void renderModel(Model*, const glm::mat4& modelMatrix, GTAObject* = nullptr, Animator* animator = nullptr);

    /**
     * @brief renderPaths renders the AI paths.
     */
    void renderPaths();
};

#endif
