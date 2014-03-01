#include <engine/Animator.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIFP.hpp>
#include <render/Model.hpp>
#include <glm/gtc/matrix_transform.hpp>

Animator::Animator()
	: animation(nullptr),  model(nullptr), time(0.f), serverTime(0.f), lastServerTime(0.f), repeat(true)
{

}

void Animator::reset()
{
	time = 0.f;
	lastServerTime= 0.f;

	if(model && animation) {
		
	}
}

void Animator::setAnimation(Animation *animation, bool repeat)
{
	if(animation == this->animation) {
		return;
	}

	this->animation = animation;
	this->repeat = repeat;
	
	reset();
}

void Animator::setModel(Model *model)
{
	if(model == this->model) {
		return;
	}

	this->model = model;

	reset();
}

void Animator::tick(float dt)
{
	if(! (animation && model)) {
		return;
	}

	lastServerTime = serverTime;
	serverTime += dt;

}

glm::vec3 Animator::getRootTranslation() const
{
	if( model && model->rootFrameIdx	!= -1 ) {
	}

	return glm::vec3();
}

glm::quat Animator::getRootRotation() const
{
	return glm::quat();
}

glm::mat4 Animator::getFrameMatrix(ModelFrame* frame, float alpha) const
{
	auto it = animation->bones.find(frame->getName());
	if(it != animation->bones.end()) {
		auto kf = it->second->getInterpolatedKeyframe(getAnimationTime(alpha));
		glm::mat4 m;
		if(it->second->type == AnimationBone::R00) {
			m = glm::translate(m, frame->getDefaultTranslation());
			m = m * glm::mat4_cast(kf.rotation);
		}
		else if(it->second->type == AnimationBone::RT0) {
			m = glm::mat4_cast(kf.rotation);
			m = glm::translate(m, kf.position);
		}
		else {
			m = glm::mat4_cast(kf.rotation);
			m = glm::translate(m, kf.position);
		}
		return m;
	}
	else {
		return frame->getTransform();
	}
}

bool Animator::isCompleted() const
{
	return serverTime >= animation->duration;
}

float Animator::getAnimationTime(float alpha) const
{
	if(repeat) {
		return fmod(serverTime + alpha, this->animation->duration);
	}
	return serverTime + alpha;
}
