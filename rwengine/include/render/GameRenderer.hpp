#ifndef _GAMERENDERER_HPP_
#define _GAMERENDERER_HPP_

class Logger;

#include <gl/gl_core_3_3.h>

#include <memory>
#include <vector>

#include <render/ViewCamera.hpp>

#include <render/OpenGLRenderer.hpp>
#include "MapRenderer.hpp"
#include "TextRenderer.hpp"
#include "WaterRenderer.hpp"

class Model;
class ModelFrame;
class GameWorld;
class GameObject;

class AreaIndicatorInfo;

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
	/** Game data to use for rendering */
	GameData* data;

	/** Logger to output messages */
	Logger* logger;

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

	// Temporary variables used during rendering
	float _renderAlpha;
	GameWorld* _renderWorld;

	/** Internal non-descript VAOs */
	GLuint vao, debugVAO;

	/** Camera values passed to renderWorld() */
	ViewCamera _camera;
	
	GLuint framebufferName;
	GLuint fbTextures[2];
	GLuint fbRenderBuffers[1];
	Renderer::ShaderProgram* postProg;

public:
	
	GameRenderer(Logger* log, GameData* data);
	~GameRenderer();
	
	/** Number of culling events */
	size_t culled;

	/** @todo Clean up all these shader program and location variables */
	Renderer::ShaderProgram* worldProg;
	Renderer::ShaderProgram* skyProg;
	Renderer::ShaderProgram* particleProg;

	GLuint ssRectProgram;
	GLint ssRectTexture, ssRectColour, ssRectSize, ssRectOffset;

	GLuint skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;

	DrawBuffer skyDbuff;
	GeometryBuffer skyGbuff;
	
	DrawBuffer cylinderBuffer;
	GeometryBuffer cylinderGeometry;

	GameData* getData() const { return data; }

    /**
	 * Renders the world using the parameters of the passed Camera.
	 * Note: The camera's near and far planes are overriden by weather effects.
	 *
	 *  - draws all objects (instances, vehicles etc.)
	 *  - draws particles
	 *  - draws water surfaces
	 *  - draws the skybox
     */
	void renderWorld(GameWorld* world, const ViewCamera &camera, float alpha);

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
	 * Renders the effects (Particles, Lighttrails etc)
	 */
	void renderEffects(GameWorld* world);

	/**
	 * @brief Draws the current on screen text.
	 */
	void drawOnScreenText();

	/** 
	 * Renders a model (who'd have thought)
	 */
	void renderModel(Model*, const glm::mat4& modelMatrix, GameObject* = nullptr, Animator* animator = nullptr);

	void renderGeometry(Model*, size_t geom, const glm::mat4& modelMatrix, float opacity, GameObject* = nullptr);
	
	/** Renders the area indicator */
	void renderAreaIndicator(const AreaIndicatorInfo* info);

	/** method for rendering AI debug information */
	void renderPaths();

	/** Increases cinematic value */
	void renderLetterbox();

	void setupRender();
	void renderPostProcess();

	Renderer* getRenderer()
	{
		return renderer;
	}
	
	void setViewport(int w, int h);
	
	MapRenderer map;
	WaterRenderer water;
	TextRenderer text;

	// Profiling data
	Renderer::ProfileInfo profObjects;
	Renderer::ProfileInfo profSky;
	Renderer::ProfileInfo profWater;
	Renderer::ProfileInfo profEffects;
};

#endif
