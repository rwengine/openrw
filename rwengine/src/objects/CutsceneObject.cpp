#include <objects/CutsceneObject.hpp>
#include <engine/Animator.hpp>

CutsceneObject::CutsceneObject(GameWorld *engine, const glm::vec3 &pos, ModelHandle *model)
	: GameObject(engine, pos, {}, model), _parent(nullptr), _bone(nullptr)
{
	animator = new Animator;
}

CutsceneObject::~CutsceneObject()
{
	delete animator;
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
