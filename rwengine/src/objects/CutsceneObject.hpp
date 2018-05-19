#ifndef _RWENGINE_CUTSCENEOBJECT_HPP_
#define _RWENGINE_CUTSCENEOBJECT_HPP_
#include <glm/glm.hpp>

#include <rw/forward.hpp>

#include <objects/GameObject.hpp>

class BaseModelInfo;
class GameWorld;
class ModelFrame;

/**
 * @brief Object type used for cutscene animations.
 */
class CutsceneObject : public GameObject, public ClumpObject {
    GameObject* _parent = nullptr;
    ModelFrame* _bone = nullptr;

public:
    CutsceneObject(GameWorld* engine, const glm::vec3& pos,
                   const glm::quat& rot, const ClumpPtr& model,
                   BaseModelInfo* modelinfo);
    ~CutsceneObject() override = default;

    Type type() const override {
        return Cutscene;
    }

    void tick(float dt) override;

    void setParentActor(GameObject* parent, ModelFrame* bone);

    GameObject* getParentActor() const {
        return _parent;
    }

    ModelFrame* getParentFrame() const {
        return _bone;
    }
};

#endif
