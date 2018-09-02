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

#if 0
    // Blend all active animations together
    std::map<ModelFrame*, BoneTransform> blendFrames;
#endif

    for (AnimationState& state : animations) {
        if (state.animation == nullptr) continue;

        if (state.boneInstances.empty()) {
            for (const auto& bone : state.animation->bones) {
                auto frame = model->findFrame(bone.first);
                if (!frame) {
                    continue;
                }
                state.boneInstances.emplace(bone.second.get(), frame);
            }
        }

        state.time = state.time + dt;

        float animTime = state.time;
        if (!state.repeat) {
            animTime = std::min(animTime, state.animation->duration);
        } else {
            animTime = std::fmod(animTime, state.animation->duration);
        }

        for (auto& b : state.boneInstances) {
            if (b.first->frames.empty()) continue;
            auto kf = b.first->getInterpolatedKeyframe(animTime);

            BoneTransform xform;
            xform.rotation = kf.rotation;
            if (b.first->type != AnimationBone::R00) {
                xform.translation = kf.position;
            }

#if 0
			auto prevAnim = blendFrames.find(b.second.frameIndex);
			if (prevAnim != blendFrames.end())
			{
				prevAnim->second.translation += xform.translation;
				prevAnim->second.rotation *= xform.rotation;
			}
			else
			{
				blendFrames[b.second.frameIndex] = xform;
			}
#else
            b.second->setTranslation(b.second->getDefaultTranslation() +
                                     xform.translation);
            b.second->setRotation(glm::mat3_cast(xform.rotation));
#endif
        }
    }

#if 0
    for (auto& p : blendFrames) {
        p.first->setTranslation(p.first->getDefaultTranslation() +
                                p.second.translation);
        p.first->setRotation(glm::mat3_cast(p.second.rotation));
    }
#endif
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
