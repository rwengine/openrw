#pragma once
#ifndef _ANIMATOR_HPP_
#define _ANIMATOR_HPP_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <map>
#include <cstdint>

class Animation;
class AnimationBone;
class Model;

/**
 * @brief The Animator class handles updating frame matricies for animations.
 */
class Animator
{
	Animation* animation;

	Model* model;

	glm::vec3 lastRootPosition;
	glm::quat lastRootRotation;

	std::vector<glm::mat4> matrices;
	std::map<int32_t, AnimationBone*> frameToBone;

	float time;
	float serverTime;
	float lastServerTime;

	void updateFrameMapping();
	void reset();

public:

	Animator();

	/**
	 * @brief setAnimation Sets the currently active animation.
	 * @param animation
	 * @todo Interpolate between the new and old frames.
	 */
	void setAnimation(Animation* animation);

	void setModel(Model* model);

	/**
	 * @brief tick Update animation paramters for server-side data.
	 * @param dt
	 */
	void tick(float dt);

	/**
	 * @brief render Update frame matricies for client-side animation.
	 * @param dt
	 */
	void render(float dt);

	/**
	 * @brief getRootTranslation Returns the translation of the root bone from the last server-side frame.
	 * @return
	 */
	glm::vec3 getRootTranslation() const;

	/**
	 * @brief getRootRotation see getRootTranslation
	 * @return
	 */
	glm::quat getRootRotation() const;

	/**
	 * @brief getFrameMatrix returns the matrix for a given frame index.
	 * @param frame
	 * @return
	 */
	glm::mat4 getFrameMatrix(size_t frame) const;
};

#endif
