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

class CharacterObject;
class VehicleObject;
class InstanceObject;
class PickupObject;

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

	struct FXParticle {
		glm::vec3 position;
		glm::vec3 direction;
		float velocity;

		enum Orientation {
			Free,
			Camera,
			UpCamera
		};
		Orientation orientation;

		float starttime;
		float lifetime;

		/// @TODO convert use of TextureInfo to a pointer so it can be used here
		GLuint texture;

		glm::vec2 size;

		glm::vec3 up;

		glm::vec3 _currentPosition;

		FXParticle(const glm::vec3& p, const glm::vec3& d, float v,
				   Orientation o, float st, float lt, GLuint texture,
				   const glm::vec2& size, const glm::vec3& up = {0.f, 0.f, 1.f})
			: position(p), direction(d), velocity(v), orientation(o),
			  starttime(st), lifetime(lt), texture(texture), size(size),
			  up(up), _currentPosition(p) {}
	};

private:
	std::vector<FXParticle> _particles;

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
	GLuint particleProgram;
	
	/// Internal VAO to avoid clobbering global state.
    GLuint vao, debugVAO;

	GLuint skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;
	
    /**
     * Renders the current World. 
     */
    void renderWorld(float alpha);

	// Object rendering methods.
	void renderPedestrian(CharacterObject* pedestrian);
	void renderVehicle(VehicleObject* vehicle);
	void renderInstance(InstanceObject* instance);
	void renderPickup(PickupObject* pickup);

	void renderWheel(Model*, const glm::mat4& matrix, const std::string& name);

	void renderItem(InventoryItem* item, const glm::mat4& modelMatrix);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, GameObject* = nullptr);

	void renderParticles();


	/** 
	 * Renders a model (who'd have thought)
	 */
	void renderModel(Model*, const glm::mat4& modelMatrix, GameObject* = nullptr, Animator* animator = nullptr);

	/**
	 * Debug method renders all AI paths
	 */
    void renderPaths();

	void addParticle(const FXParticle& particle);

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
