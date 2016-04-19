#ifndef _RWENGINE_OBJECTRENDERER_HPP_
#define _RWENGINE_OBJECTRENDERER_HPP_

#include <glm/glm.hpp>
#include <rw/types.hpp>
#include <render/ViewCamera.hpp>
#include <render/OpenGLRenderer.hpp>
#include <objects/GameObject.hpp>
#include <engine/GameWorld.hpp>
#include <gl/DrawBuffer.hpp>


/*
Rendering Instruction contents:
	Model matrix
	List of subgeometries(??)
*/
typedef uint64_t RenderKey;
struct RenderInstruction
{
	RenderKey sortKey;
	// Ideally, this would just be an index into a buffer that contains the matrix
	glm::mat4 model;
	DrawBuffer* dbuff;
	Renderer::DrawParameters drawInfo;

	RenderInstruction(
			RenderKey key,
			const glm::mat4& model,
			DrawBuffer* dbuff,
			const Renderer::DrawParameters& dp)
		: sortKey(key)
		, model(model)
		, dbuff(dbuff)
		, drawInfo(dp)
	{

	}
};
typedef std::vector<RenderInstruction> RenderList;

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
