#ifndef _GAMERENDERER_HPP_
#define _GAMERENDERER_HPP_

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
class InventoryItem;

/**
 * Renderer 
 * 
 * Handles low level rendering of Models, as well as high level rendering of
 * objects in the world.
 */
class GameRenderer
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

	float _renderAlpha;

public:
	
	GameRenderer(GameWorld*);
	
	ViewCamera camera;
	
	/// The numer of things rendered by the last renderWorld
	size_t rendered;
	size_t culled;

	/* TODO clean up all these variables */
	GLuint worldProgram;
	GLint uniTexture;
	GLuint ubiScene, ubiObject;
	GLuint uboScene, uboObject;

	GLuint skyProgram;
	GLuint waterProgram, waterMVP, waterHeight, waterTexture, waterSize, waterTime, waterPosition, waterWave;
	GLint skyUniView, skyUniProj, skyUniTop, skyUniBottom;
	
	/// Internal VAO to avoid clobbering global state.
    GLuint vao, debugVAO;
	
	GLuint skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;
	
    /**
     * Renders the current World. 
     */
    void renderWorld(float alpha);

	void renderWheel(Model*, const glm::mat4& matrix, const std::string& name);

	void renderItem(InventoryItem* item, const glm::mat4& modelMatrix);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, GameObject* = nullptr);

	/** 
	 * Renders a model (who'd have thought)
	 */
	void renderModel(Model*, const glm::mat4& modelMatrix, GameObject* = nullptr, Animator* animator = nullptr);

    /**
     * Debug method renders all AI paths
     */
    void renderPaths();

	static GLuint currentUBO;
	template<class T> void uploadUBO(GLuint buffer, const T& data)
	{
		if( currentUBO != buffer ) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffer);
			currentUBO = buffer;
		}
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &data, GL_DYNAMIC_DRAW);
	}
};

struct SceneUniformData {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 ambient;
	glm::vec4 dynamic;
	float fogStart;
	float fogEnd;
};

struct ObjectUniformData {
	glm::mat4 model;
	glm::vec4 colour;
	float diffuse;
	float ambient;
};

#endif
