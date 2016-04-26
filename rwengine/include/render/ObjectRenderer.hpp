#ifndef _RWENGINE_OBJECTRENDERER_HPP_
#define _RWENGINE_OBJECTRENDERER_HPP_

#include <glm/glm.hpp>
#include <rw/types.hpp>
#include <render/ViewCamera.hpp>
#include <render/OpenGLRenderer.hpp>
#include <objects/GameObject.hpp>
#include <engine/GameWorld.hpp>
#include <gl/DrawBuffer.hpp>

/**
 * @brief The ObjectRenderer class handles object -> renderer transformation
 *
 * Determines what parts of an object are within a camera frustum and exports
 * a list of things to render for the object.
 */
class ObjectRenderer
{
public:
	/**
	 * @brief buildRenderList
	 *
	 * Exports rendering instructions for an object
	 */
	static void buildRenderList(GameWorld* world, GameObject* object,
								const ViewCamera& camera, float renderAlpha,
								RenderList& outList);
};

#endif
