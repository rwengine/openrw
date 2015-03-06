#include <objects/CutsceneObject.hpp>
#include <engine/Animator.hpp>
#include <data/Skeleton.hpp>

CutsceneObject::CutsceneObject(GameWorld *engine, const glm::vec3 &pos, const ModelRef& model)
	: GameObject(engine, pos, {}, model), _parent(nullptr), _bone(nullptr)
{
	skeleton = new Skeleton;
	animator = new Animator(model->resource, skeleton);
}

CutsceneObject::~CutsceneObject()
{
}

void CutsceneObject::tick(float dt)
{
	animator->tick(dt);
}

void CutsceneObject::setParentActor(GameObject *parent, ModelFrame *bone)
{
	_parent = parent;
	_bone = bone;
}
