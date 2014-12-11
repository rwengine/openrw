#pragma once
#ifndef _ANIMATOR_HPP_
#define _ANIMATOR_HPP_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <map>
#include <loaders/LoaderIFP.hpp>
#include <cstdint>

class Model;
class ModelFrame;

class Skeleton;

/**
 * @brief calculates animation frame matrices, as well as procedural frame
 * animation.
 */
class Animator
{
	/**
	 * @brief _animations Queue of animations to play.
	 */
	std::queue<Animation*> _animations;

	/**
	 * @brief model The model being animated.
	 */
	Model* model;
	
	/**
	 * @brief Skeleton instance.
	 */
	Skeleton* skeleton;

	/**
	 * @brief Stores data required to animate a model frame
	 */
	struct BoneInstanceData
	{
		unsigned int frameIdx;
	};

	std::map<AnimationBone*, BoneInstanceData> boneInstances;

	// Used in determining how far the skeleton being animated has moved
	// From it's local origin.
	glm::vec3 lastRootPosition;
	glm::quat lastRootRotation;

	float time;
	float serverTime;
	float lastServerTime;

	bool playing;
	bool repeat;

	void reset();

public:

	Animator(Model* model, Skeleton* skeleton);

	/**
	 * @brief setAnimation Sets the currently active animation.
	 * @param animation
	 * @param repeat If true animation will restart after ending.
	 * @todo Interpolate between the new and old frames.
	 */
	void setAnimation(Animation* animation, bool repeat = true);

	void queueAnimation(Animation* animation);

	void next();

	const std::queue<Animation*> getAnimationQueue() const
	{ return _animations; }

	const Animation* getAnimation() const
	{ return _animations.empty() ? nullptr : _animations.front(); }

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
	 * @brief getTimeTranslation returns the translation of the root bone at the current time.
	 * @return
	 */
	glm::vec3 getTimeTranslation() const;

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
	void setAnimationTime(float time);

	void setPlaying( bool play ) { playing = play; }
};

#endif
