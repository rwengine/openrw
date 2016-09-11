#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <objects/CutsceneObject.hpp>

CutsceneObject::CutsceneObject(GameWorld *engine, const glm::vec3 &pos,
                               const glm::quat &rot,
                               const ModelRef &model, BaseModelInfo *modelinfo)
    : GameObject(engine, pos, rot, modelinfo, model)
    , _parent(nullptr)
    , _bone(nullptr) {
    skeleton = new Skeleton;
    animator = new Animator(model->resource, skeleton);
}

CutsceneObject::~CutsceneObject() {
}

void CutsceneObject::tick(float dt) {
    animator->tick(dt);
}

void CutsceneObject::setParentActor(GameObject *parent, ModelFrame *bone) {
    _parent = parent;
    _bone = bone;
}
