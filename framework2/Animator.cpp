#include <renderwure/engine/Animator.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/render/Model.hpp>

Animator::Animator()
	: animation(nullptr),  model(nullptr), time(0.f), serverTime(0.f), lastServerTime(0.f), repeat(true)
{

}

void Animator::updateFrameMapping()
{
	if(! (animation && model) ) {
		return;
	}

	frameToBone.clear();

	for( size_t f = 0; f < model->frames.size(); ++f ) {
		AnimationBone* bone = animation->bones[model->frameNames[f]];
		if(bone) {
			frameToBone.insert({f, bone});
		}
	}
}

void Animator::reset()
{
	time = 0.f;
	lastServerTime= 0.f;

	if(model && animation) {
		if(model->rootFrameIdx != -1) {
			auto it = frameToBone.find(model->rootFrameIdx);
			if( it != frameToBone.end() )
			{
				AnimationBone* b = it->second;
				AnimationKeyframe& KF = b->frames[0];
				if(b->type == AnimationBone::RT0 || b->type == AnimationBone::RTS) {
					lastRootPosition = KF.position;
				}
				lastRootRotation = KF.rotation;
			}
		}
	}
}

void Animator::setAnimation(Animation *animation, bool repeat)
{
	if(animation == this->animation) {
		return;
	}

	this->animation = animation;
	this->repeat = repeat;

	updateFrameMapping();
	reset();
}

void Animator::setModel(Model *model)
{
	if(model == this->model) {
		return;
	}

	this->model	= model;

	if(model != nullptr) {
		matrices.resize(model->frames.size());
	}

	updateFrameMapping();
	reset();
}

void Animator::tick(float dt)
{
	if(! (animation && model)) {
		return;
	}

	// Calculate the root node movement.
	if( model->rootFrameIdx	!= -1 ) {
		auto it = frameToBone.find(model->rootFrameIdx);
		if( it != frameToBone.end() )
		{
			AnimationBone* boneanim = frameToBone[model->rootFrameIdx];
			if( boneanim->type != AnimationBone::R00 ) {
				auto keyframe = boneanim->getInterpolatedKeyframe(serverTime);
				lastRootPosition = keyframe.position;
			}
		}
	}

	lastServerTime = serverTime;
	serverTime += dt;

	for( size_t fi = 0; fi < model->frames.size(); ++fi ) {
		glm::mat4 fmat = glm::mat4(model->frames[fi].defaultRotation);
		fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);

		if( animation && fi < model->frameNames.size() ) {
			if(repeat) {
				serverTime = fmod(serverTime, animation->duration);
			}
			else {
				serverTime = std::min(serverTime, animation->duration);
			}
			
			if( lastServerTime > serverTime ) {
				lastRootPosition = glm::vec3(0.f);
			}

			auto it = frameToBone.find(fi);
			if( it == frameToBone.end() ) continue;
			AnimationBone* boneanim = it->second;

			if( boneanim && boneanim->frames.size() > 0 ) {
				auto keyframe = boneanim->getInterpolatedKeyframe(serverTime);
				fmat = glm::mat4(1.0f) * glm::mat4_cast(keyframe.rotation);

				// Only add the translation back if is is not present.
				if( boneanim->type == AnimationBone::R00 ) {
					fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);
				}
				else {
					if( fi == model->rootFrameIdx ) {
						// Ignore root translation.
						fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);
					}
					else {
						fmat[3] = glm::vec4(keyframe.position, 1.f);
					}
				}
			}
		}

		matrices[fi] = fmat;
	}
}

void Animator::render(float dt)
{

}

glm::vec3 Animator::getRootTranslation() const
{
	if( model && model->rootFrameIdx	!= -1 ) {
		auto it = frameToBone.find(model->rootFrameIdx);
		if( it != frameToBone.end() )
		{
			AnimationBone* boneanim = it->second;

			if( boneanim->type != AnimationBone::R00 ) {
				auto keyframe = boneanim->getInterpolatedKeyframe(serverTime);
				return keyframe.position - lastRootPosition;
			}
		}
	}

	return glm::vec3();
}

glm::quat Animator::getRootRotation() const
{
	glm::quat();
}

glm::mat4 Animator::getFrameMatrix(size_t frame) const
{
	if( model ) {
		Model::Frame& f = model->frames[frame];
		if( f.parentFrameIndex == -1 ) {
			return matrices.at(frame);
		}
		else {
			return getFrameMatrix(f.parentFrameIndex) * matrices.at(frame);
		}
	}
}

bool Animator::isCompleted() const
{
	return serverTime >= animation->duration;
}