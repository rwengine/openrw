#include <data/Clump.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <loaders/LoaderDFF.hpp>

Animator::Animator(Clump* model, Skeleton* skeleton)
    : model(model), skeleton(skeleton) {
}

void Animator::tick(float dt) {
    if (model == nullptr || animations.empty()) {
        return;
    }

    struct BoneTransform {
        glm::vec3 translation;
        glm::quat rotation;
    };

    // Blend all active animations together
    std::map<unsigned int, BoneTransform> blendFrames;

    for (AnimationState& state : animations) {
        RW_CHECK(state.animation != nullptr,
                 "AnimationState with no animation");
        if (state.animation == nullptr) continue;

        if (state.boneInstances.size() == 0) {
            for (unsigned int f = 0; f < model->frames.size(); ++f) {
                auto bit =
                    state.animation->bones.find(model->frames[f]->getName());
                if (bit != state.animation->bones.end()) {
                    state.boneInstances.insert({bit->second, {f}});
                }
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
            blendFrames[b.second.frameIndex] = xform;
#endif
        }
    }

    for (auto& p : blendFrames) {
        auto& data = skeleton->getData(p.first);
        Skeleton::FrameData fd;
        fd.b = data.a;
        fd.enabled = data.enabled;

        fd.a.translation = model->frames[p.first]->getDefaultTranslation() +
                           p.second.translation;
        fd.a.rotation = p.second.rotation;

        skeleton->setData(p.first, fd);
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
