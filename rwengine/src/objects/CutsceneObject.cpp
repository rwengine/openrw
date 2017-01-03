#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <objects/CutsceneObject.hpp>

CutsceneObject::CutsceneObject(GameWorld *engine, const glm::vec3 &pos,
                               const glm::quat &rot, Clump *model,
                               BaseModelInfo *modelinfo)
    : GameObject(engine, pos, rot, modelinfo)
    , _parent(nullptr)
    , _bone(nullptr) {
    if (model) {
        setModel(model);
    }
    else {
        setModel(getModelInfo<ClumpModelInfo>()->getModel());
    }
    skeleton = new Skeleton;
    animator = new Animator(getModel(), skeleton);
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
