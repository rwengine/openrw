#pragma once
#ifndef _GTAINSTANCE_HPP_
#define _GTAINSTANCE_HPP_
#include <renderwure/engine/GTAObject.hpp>

/**
 * @struct GTAInstance
 *  Stores references to the Object data and the instance
 */
struct GTAInstance : public GTAObject
{
	glm::vec3 scale;
	std::shared_ptr<LoaderIDE::OBJS_t> object;
	std::shared_ptr<GTAInstance> LODinstance;

	GTAInstance(
		GTAEngine* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		Model* model,
		const glm::vec3& scale,
		std::shared_ptr<LoaderIDE::OBJS_t> obj,
		std::shared_ptr<GTAInstance> lod
			);

	Type type() { return Instance; }
};


#endif
