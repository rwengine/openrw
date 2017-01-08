#include <data/Clump.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <loaders/LoaderDFF.hpp>
#include <queue>

Animator::Animator(Clump* model) : model(model) {
}

void Animator::tick(float dt) {
    if (model == nullptr || animations.empty()) {
        return;
    }

    struct BoneTransform {
        glm::vec3 translation;
        glm::quat rotation;
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
                state.boneInstances.insert({bone.second, frame});
            }
        }

        state.time = state.time + dt;

        float animTime = state.time;
        if (!state.repeat) {
            animTime = std::min(animTime, state.animation->duration);
        } else {
            animTime = fmod(animTime, state.animation->duration);
        }

        for (auto& b : state.boneInstances) {
            if (b.first->frames.size() == 0) continue;
            auto kf = b.first->getInterpolatedKeyframe(animTime);

            BoneTransform xform;
            if (b.first->type == AnimationBone::R00) {
                xform.rotation = kf.rotation;
            } else if (b.first->type == AnimationBone::RT0) {
                xform.rotation = kf.rotation;
                xform.translation = kf.position;
            } else {
                xform.rotation = kf.rotation;
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
