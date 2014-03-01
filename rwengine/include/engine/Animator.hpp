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
class ModelFrame;

/**
 * @brief The Animator class handles updating frame matricies for animations.
 */
class Animator
{
	Animation* animation;

	Model* model;

	glm::vec3 lastRootPosition;
	glm::quat lastRootRotation;

	float time;
	float serverTime;
	float lastServerTime;
	
	bool repeat;

	void reset();

public:

	Animator();

	/**
	 * @brief setAnimation Sets the currently active animation.
	 * @param animation
	 * @param repeat If true animation will restart after ending.
	 * @todo Interpolate between the new and old frames.
	 */
	void setAnimation(Animation* animation, bool repeat = true);

	void setModel(Model* model);

	/**
	 * @brief getFrameMatrix returns the matrix for frame at the current time
	 * @param t
	 * @param frame
	 * @return
	 */
	glm::mat4 getFrameMatrix(ModelFrame* frame, float alpha) const;

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
	 * Returns true if the animation has finished playing.
	 */
	bool isCompleted() const; 

	float getAnimationTime(float alpha = 0.f) const;
};

#endif
