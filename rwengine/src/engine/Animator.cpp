#include "engine/Animator.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <data/Clump.hpp>

#include "loaders/LoaderIFP.hpp"

#include <algorithm>
#include <cmath>

Animator::Animator(const ClumpPtr& _model) : model(_model) {
}

void Animator::tick(float dt) {
    if (model == nullptr || animations.empty()) {
        return;
    }

    struct BoneTransform {
        glm::vec3 translation{};
        glm::quat rotation{1.0f,0.0f,0.0f,0.0f};
    };

    for (AnimationState& state : animations) {
        if (state.animation == nullptr) continue;

        if (state.boneInstances.empty()) {
            for (const auto& [name, bonePtr] : state.animation->bones) {
                auto frame = model->findFrame(name);
                if (!frame) {
                    continue;
                }
                state.boneInstances.emplace(bonePtr.get(), frame);
            }
        }

        state.time = state.time + dt;

        float animTime = state.time;
        if (!state.repeat) {
            animTime = std::min(animTime, state.animation->duration);
        } else {
            animTime = std::fmod(animTime, state.animation->duration);
        }

        for (auto& [bonePtr, frame] : state.boneInstances) {
            if (bonePtr->frames.empty()) continue;
            auto kf = bonePtr->getInterpolatedKeyframe(animTime);

            BoneTransform xform;
            xform.rotation = kf.rotation;
            if (bonePtr->type != AnimationBone::R00) {
                xform.translation = kf.position;
            }
            frame->setTranslation(frame->getDefaultTranslation() +
                                     xform.translation);
            frame->setRotation(glm::mat3_cast(xform.rotation));
        }
    }
}

bool Animator::isCompleted(unsigned int slot) const {
    if (slot < animations.size()) {
        return animations[slot].animation
                   ? animations[slot].time >=
                         animations[slot].animation->duration
                   : true;
    }
    return false;
}

float Animator::getAnimationTime(unsigned int slot) const {
    if (slot < animations.size()) {
        return animations[slot].time;
    }
    return 0.f;
}

void Animator::setAnimationTime(unsigned int slot, float time) {
    if (slot < animations.size()) {
        animations[slot].time = time;
    }
}
