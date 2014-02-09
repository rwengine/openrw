#include <engine/Animator.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIFP.hpp>
#include <render/Model.hpp>

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

void Animator::render(float dt)
{

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

glm::mat4 Animator::getFrameMatrix(size_t frame) const
{
	return glm::mat4();
}

bool Animator::isCompleted() const
{
	return serverTime >= animation->duration;
}