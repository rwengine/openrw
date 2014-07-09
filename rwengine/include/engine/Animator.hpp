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
	 * @brief Stores data required to animate a model frame
	 */
	struct FrameInstanceData {
		bool visible;
		AnimationBone* bone;
		// Used if bone is non-null.
		AnimationKeyframe first;
		AnimationKeyframe second;
		// Used if bone is null and entry exists.
		glm::quat orientation;

		/// Construct from animation data
		FrameInstanceData(AnimationBone* bone, const AnimationKeyframe& a, const AnimationKeyframe& b)
			: visible(true), bone(bone), first(a), second(b) {}

		/// Construct from procedural data
		FrameInstanceData(bool visible, const glm::quat& orientation = {})
			: visible(visible), bone(nullptr), orientation(orientation) {}
	};

	std::map<ModelFrame*, FrameInstanceData> _frameInstances;

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

	Animator();

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

	void setModel(Model* model);

	void setFrameVisibility(ModelFrame* frame, bool visible);
	bool getFrameVisibility(ModelFrame* frame) const;

	void setFrameOrientation(ModelFrame* frame, const glm::quat& orientation);
	glm::quat getFrameOrientation(ModelFrame* frame) const;

	FrameInstanceData* getFrameInstance(ModelFrame* frame);

	/**
	 * @brief getFrameMatrix returns the matrix for frame at the given time
	 * @param t
	 * @param frame
	 * @return
	 */
	glm::mat4 getFrameMatrixAt(ModelFrame* frame, float time, bool disableRoot = true) const;
	AnimationKeyframe getKeyframeAt(ModelFrame* frame, float time) const;

	glm::mat4 getFrameMatrix(ModelFrame* frame, float alpha = 0.f, bool ignoreRoot = true) const;

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
	void setAnimationTime(float time);

	void setPlaying( bool play ) { playing = play; }
};

#endif
