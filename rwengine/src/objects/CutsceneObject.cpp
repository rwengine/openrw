#include "objects/CutsceneObject.hpp"

#include <data/Clump.hpp>

#include "data/ModelData.hpp"
#include "engine/Animator.hpp"

CutsceneObject::CutsceneObject(GameWorld *engine, const glm::vec3 &pos,
                               const glm::quat &rot, const ClumpPtr& model,
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
    setClump(ClumpPtr(getModel()->clone()));
    animator = new Animator(getClump());
}

CutsceneObject::~CutsceneObject() = default;

void CutsceneObject::tick(float dt) {
    animator->tick(dt);
}

void CutsceneObject::setParentActor(GameObject *parent, ModelFrame *bone) {
    _parent = parent;
    _bone = bone;
}
