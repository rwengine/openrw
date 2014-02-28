#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <memory>
#include <vector>

#include <render/ViewCamera.hpp>

class Model;
class ModelFrame;
class GameWorld;
class GameObject;
class Animator;

/**
 * Renderer 
 * 
 * Handles low level rendering of Models, as well as high level rendering of
 * objects in the world.
 */
class GTARenderer
{
    GameWorld* engine;
	
	struct RQueueEntry {
		Model* model;
		size_t g;
		size_t sg;
		glm::mat4 matrix;
		GameObject* object;
	};
	
	bool renderFrame(Model* m, ModelFrame* f, const glm::mat4& matrix, GameObject* object, bool queueTransparent = true);
	
	// Internal method for processing sub-geometry
	bool renderSubgeometry(Model* model, size_t g, size_t sg, const glm::mat4& matrix, GameObject* object, bool queueTransparent = true);
	
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

	/* TODO clean up all these variables */
	GLint uniModel, uniProj, uniView, uniCol, uniAmbientCol, uniSunDirection, uniDynamicCol;
	GLint uniMatDiffuse, uniMatAmbient, uniFogStart, uniFogEnd;
	GLuint worldProgram;
	GLuint skyProgram;
	GLint skyUniView, skyUniProj, skyUniTop, skyUniBottom;
	
	/// Internal VAO to avoid clobbering global state.
    GLuint vao, debugVAO;
	
    GLuint planeVBO, skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;
	
    /**
     * Renders the current World. 
     */
    void renderWorld();

	void renderNamedFrame(Model*, const glm::mat4& matrix, const std::string& name);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, GameObject* = nullptr);

	/** 
	 * Renders a model (who'd have thought)
	 */
	void renderModel(Model*, const glm::mat4& modelMatrix, GameObject* = nullptr, Animator* animator = nullptr);

    /**
     * Debug method renders all AI paths
     */
    void renderPaths();
};

#endif
