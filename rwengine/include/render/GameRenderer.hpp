#ifndef _GAMERENDERER_HPP_
#define _GAMERENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <memory>
#include <vector>

#include <render/ViewCamera.hpp>

#include <render/OpenGLRenderer.hpp>

class Model;
class ModelFrame;
class GameWorld;
class GameObject;

/// @todo migrate to some other way of rendering each object type.
class CharacterObject;
class VehicleObject;
class InstanceObject;
class PickupObject;
class ProjectileObject;
class CutsceneObject;

class Animator;
class InventoryItem;

class Renderer;

/**
 * @brief Implements high level drawing logic and low level draw commands
 *
 * Rendering of object types is handled by drawWorld, calling the respective
 * render function for each object.
 */
class GameRenderer
{
	/** Pointer to the world instance */
	GameWorld* engine;

	/** The low-level drawing interface to use */
	Renderer* renderer;
	
	/** Stores data for deferring transparent objects */
	struct RQueueEntry {
		Model* model;
		size_t g;
		size_t sg;
		glm::mat4 matrix;
		Renderer::DrawParameters dp;
		GameObject* object;
	};
	
	/**
	 * @brief renders a model's frame.
	 * @param m
	 * @param f
	 * @param matrix
	 * @param object
	 * @param queueTransparent abort the draw if the frame contains transparent materials
	 * @return True if the frame was drawn, false if it should be queued
	 */
	bool renderFrame(Model* m, ModelFrame* f, const glm::mat4& matrix, GameObject* object, float opacity, bool queueTransparent = true);

	/** Transparent objects are queued into this list */
	std::vector<RQueueEntry> transparentDrawQueue;

	float _renderAlpha;

	/** Internal non-descript VAOs */
	GLuint vao, debugVAO;

public:

	/**
	 * @brief Stores particle effect instance data
	 */
	struct FXParticle {

		/** Initial world position */
		glm::vec3 position;

		/** Direction of particle */
		glm::vec3 direction;

		/** Velocity of particle */
		float velocity;

		/** Particle orientation modes */
		enum Orientation {
			Free, /** faces direction using up */
			Camera, /** Faces towards the camera @todo implement */
			UpCamera /** Face closes point in camera's look direction */
		};
		Orientation orientation;

		/** Game time at particle instantiation */
		float starttime;
		float lifetime;

		/** Texture name */
		GLuint texture;

		/** Size of particle */
		glm::vec2 size;

		/** Up direction (only used in Free mode) */
		glm::vec3 up;

		/** Render tint colour */
		glm::vec4 colour;

		/** Internal cache value */
		glm::vec3 _currentPosition;

		/** Constructs a particle */
		FXParticle(const glm::vec3& p, const glm::vec3& d, float v,
				   Orientation o, float st, float lt, GLuint texture,
				   const glm::vec2& size, const glm::vec3& up = {0.f, 0.f, 1.f},
				   const glm::vec4& colour = {1.f, 1.f, 1.f, 1.f})
			: position(p), direction(d), velocity(v), orientation(o),
			  starttime(st), lifetime(lt), texture(texture), size(size),
			  up(up), colour(colour), _currentPosition(p) {}
	};

private:

	/** Particles in flight */
	std::vector<FXParticle> _particles;

	/** Camera values passed to renderWorld() */
	ViewCamera _camera;

public:
	
	GameRenderer(GameWorld*);
	
	/** Number of issued draw calls */
	size_t rendered;
	/** Number of culling events */
	size_t culled;
	size_t frames;
	size_t geoms;

	/** @todo Clean up all these shader program and location variables */
	Renderer::ShaderProgram* worldProg;
	Renderer::ShaderProgram* skyProg;
	Renderer::ShaderProgram* waterProg;

	GLuint particleProgram;

	GLuint ssRectProgram;
	GLint ssRectTexture, ssRectColour, ssRectSize, ssRectOffset;

	GLuint skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;

	DrawBuffer skyDbuff;
	GeometryBuffer skyGbuff;
	
    /**
	 * Renders the world using the parameters of the passed Camera.
	 * Note: The camera's near and far planes are overriden by weather effects.
	 *
	 *  - draws all objects (instances, vehicles etc.)
	 *  - draws particles
	 *  - draws water surfaces
	 *  - draws the skybox
     */
	void renderWorld(const ViewCamera &camera, float alpha);

	/**
	 * @brief draws a CharacterObject and any item they are holding.
	 * @param pedestrian the character to render
	 */
	void renderPedestrian(CharacterObject* pedestrian);

	/**
	 * @brief draws a VehicleObject and it's wheels.
	 * @param vehicle vehicle to render
	 */
	void renderVehicle(VehicleObject* vehicle);

	/**
	 * @brief draw part of the world.
	 */
	void renderInstance(InstanceObject* instance);

	/**
	 * @brief draws a pickup with it's model
	 * @param pickup
	 * @todo corona rendering, with tint.
	 */
	void renderPickup(PickupObject* pickup);

	void renderProjectile(ProjectileObject* projectile);

	void renderCutsceneObject(CutsceneObject *cutscene);

	void renderWheel(Model*, const glm::mat4& matrix, const std::string& name);

	void renderItem(InventoryItem* item, const glm::mat4& modelMatrix);

	/**
	 * @brief renders all visible particles and removes expired
	 */
	void renderParticles();

	/**
	 * @brief Draws the current on screen text.
	 */
	void drawOnScreenText();

	/** 
	 * Renders a model (who'd have thought)
	 */
	void renderModel(Model*, const glm::mat4& modelMatrix, GameObject* = nullptr, Animator* animator = nullptr);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, float opacity, GameObject* = nullptr);

	/** method for rendering AI debug information */
	void renderPaths();

	/** Increases cinematic value */
	void renderLetterbox();

	/** Adds a particle to the rendering */
	void addParticle(const FXParticle& particle);
};

#endif
