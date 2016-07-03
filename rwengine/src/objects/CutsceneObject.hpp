#pragma once
#ifndef _CUTSCENEOBJECT_HPP_
#define _CUTSCENEOBJECT_HPP_
#include <objects/GameObject.hpp>

/**
 * @brief Object type used for cutscene animations.
 */
class CutsceneObject : public GameObject
{
	GameObject* _parent;
	ModelFrame* _bone;

public:

	CutsceneObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model);
	~CutsceneObject();

	Type type() { return Cutscene; }

	void tick(float dt);

	void setParentActor(GameObject* parent, ModelFrame* bone);

	GameObject* getParentActor() const
	{
		return _parent;
	}

	ModelFrame* getParentFrame() const
	{
		return _bone;
	}
};

#endif
