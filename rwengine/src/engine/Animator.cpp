#include <engine/Animator.hpp>
#include <loaders/LoaderDFF.hpp>
#include <data/Model.hpp>
#include <data/Skeleton.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Animator::Animator(Model* model, Skeleton* skeleton)
	: model(model), skeleton(skeleton), time(0.f), serverTime(0.f),
	lastServerTime(0.f), playing(true), repeat(true)
{
	reset();
}

void Animator::reset()
{
	time = 0.f;
	serverTime = 0.f;
	lastServerTime = 0.f;

	boneInstances.clear();
	
	if( getAnimation() )
	{
		for( unsigned int f = 0; f < model->frames.size(); ++f )
		{
			auto bit = getAnimation()->bones.find( model->frames[f]->getName() );
			if( bit != getAnimation()->bones.end() )
			{
				boneInstances.insert( { bit->second, { f } } );
			}
		}
	}
}

void Animator::setAnimation(Animation *animation, bool repeat)
{
	if(!_animations.empty() && animation == _animations.front()) {
		return;
	}

	while(!_animations.empty()) _animations.pop();
	queueAnimation(animation);
	this->repeat = repeat;

	//_frameInstances.clear();
	
	reset();
}

void Animator::queueAnimation(Animation *animation)
{
	_animations.push(animation);
}

void Animator::next()
{
	_animations.pop();
	reset();
}

void Animator::tick(float dt)
{
	if( model == nullptr || _animations.empty() ) {
		return;
	}

	if( playing ) {
		lastServerTime = serverTime;
		serverTime += dt;
	}
	
	for( auto& b : boneInstances )
	{
		auto kf = b.first->getInterpolatedKeyframe(getAnimationTime(1.f));
		
		auto& data = skeleton->getData(b.second.frameIdx);
		ModelFrame* frame = model->frames[b.second.frameIdx];
		
		Skeleton::FrameData fd;
		
		fd.b = data.a;
		
		if(b.first->type == AnimationBone::R00 ) {
			fd.a.rotation = kf.rotation;
			fd.a.translation = frame->getDefaultTranslation();
		}
		else if(b.first->type == AnimationBone::RT0) {
			fd.a.rotation = kf.rotation;
			fd.a.translation = kf.position;
		}
		else {
			fd.a.rotation = kf.rotation;
			fd.a.translation = kf.position;
		}
		
		fd.enabled = data.enabled;
		
		skeleton->setData(b.second.frameIdx, fd);
	}

	if( isCompleted() && ! repeat && _animations.size() > 1 ) {
		next();
	}
}

glm::vec3 Animator::getRootTranslation() const
{
	if(!model->frames[model->rootFrameIdx]->getChildren().empty() && !_animations.empty()) {
		ModelFrame* realRoot = model->frames[model->rootFrameIdx]->getChildren()[0];

		auto it = getAnimation()->bones.find(realRoot->getName());
		if(it != getAnimation()->bones.end()) {
			auto start = it->second->frames.front().position;
			auto end = it->second->frames.back().position;
			return end - start;
		}
	}

	return glm::vec3();
}

glm::vec3 Animator::getTimeTranslation(float alpha) const
{
	if(!model->frames[model->rootFrameIdx]->getChildren().empty() && !_animations.empty()) {
		ModelFrame* realRoot = model->frames[model->rootFrameIdx]->getChildren()[0];

		auto it = getAnimation()->bones.find(realRoot->getName());
		if(it != getAnimation()->bones.end()) {
			auto start = it->second->frames.front().position;
			auto end = it->second->frames.back().position;
			return glm::mix(start, end, (getAnimationTime(alpha) / getAnimation()->duration) );
		}
	}

	return glm::vec3();
}

glm::quat Animator::getRootRotation() const
{
	return glm::quat();
}

bool Animator::isCompleted() const
{
	return getAnimation() ? serverTime >= getAnimation()->duration : true;
}

float Animator::getAnimationTime(float alpha) const
{
	float td = serverTime - lastServerTime;
	if(repeat) {
		float t = serverTime + td * alpha;
		while( t > getAnimation()->duration )
		{
			t -= getAnimation()->duration;
		}
		return t;
	}
	return serverTime + td * alpha;
}

void Animator::setAnimationTime(float time)
{
	lastServerTime = serverTime;
	serverTime = time;

}
