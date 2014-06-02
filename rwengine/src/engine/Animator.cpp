#include <engine/Animator.hpp>
#include <loaders/LoaderDFF.hpp>
#include <render/Model.hpp>
#include <glm/gtc/matrix_transform.hpp>

Animator::Animator()
	: model(nullptr), time(0.f), serverTime(0.f), lastServerTime(0.f), repeat(true)
{

}

void Animator::reset()
{
	time = 0.f;
	serverTime = 0.f;
	lastServerTime = 0.f;

	if( _boneMatrices.empty() ) {
		if( ! getAnimation() || ! model ) return;

		for( ModelFrame* f : model->frames ) {
			auto it = getAnimation()->bones.find(f->getName());
			if( it == getAnimation()->bones.end() ) continue;

			auto A = getKeyframeAt(f, 0.f);
			_boneMatrices[f] = { it->second, A, A };
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

	_boneMatrices.clear();
	
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

void Animator::setModel(Model *model)
{
	if(model == this->model) {
		return;
	}

	this->model = model;

	_boneMatrices.clear();

	reset();
}

void Animator::tick(float dt)
{
	if( model == nullptr || _animations.empty() ) {
		return;
	}

	lastServerTime = serverTime;
	serverTime += dt;

	for( auto& p : _boneMatrices ) {
		p.second.second = p.second.first;
		float t = getAnimationTime();
		p.second.first = getKeyframeAt(p.first, t);
	}

	if( isCompleted() && ! repeat && _animations.size() > 1 ) {
		next();
	}
}

glm::vec3 Animator::getRootTranslation() const
{
	// This is a pretty poor assumption.
	if(!model->frames[model->rootFrameIdx]->getChildren().empty()
			&& !_animations.empty()) {
		ModelFrame* realRoot = model->frames[model->rootFrameIdx]->getChildren()[0];
		auto it = getAnimation()->bones.find(realRoot->getName());
		if(it != getAnimation()->bones.end()) {
			float df = fmod(lastServerTime, getAnimation()->duration);
			float rt = getAnimationTime();
			if(df < rt) {
				auto lastKF = it->second->getInterpolatedKeyframe(df);
				auto KF = it->second->getInterpolatedKeyframe(rt);
				return KF.position - lastKF.position;
			}
		}
	}

	return glm::vec3();
}

glm::quat Animator::getRootRotation() const
{
	return glm::quat();
}

glm::mat4 Animator::getFrameMatrixAt(ModelFrame* frame, float time, bool disableRoot) const
{
	if(getAnimation()) {
		auto it = getAnimation()->bones.find(frame->getName());
		if(it != getAnimation()->bones.end()) {
			auto kf = it->second->getInterpolatedKeyframe(time);
			glm::mat4 m;
			bool isRoot = frame->getParent() ? !! frame->getParent() : false;
			if(it->second->type == AnimationBone::R00 || ( isRoot && disableRoot ) ) {
				m = glm::translate(m, frame->getDefaultTranslation());
				m = m * glm::mat4_cast(kf.rotation);
			}
			else if(it->second->type == AnimationBone::RT0) {
				m = glm::translate(m, kf.position);
				m = m * glm::mat4_cast(kf.rotation);
			}
			else {
				m = glm::translate(m, kf.position);
				m = m * glm::mat4_cast(kf.rotation);
			}
			return m;
		}
	}

	return frame->getTransform();
}

AnimationKeyframe Animator::getKeyframeAt(ModelFrame *frame, float time) const
{
	if(getAnimation()) {
		auto it = getAnimation()->bones.find(frame->getName());
		if(it != getAnimation()->bones.end()) {
			return it->second->getInterpolatedKeyframe(time);
		}
	}
	return { glm::toQuat(frame->getDefaultRotation()), frame->getDefaultTranslation(), glm::vec3(1.f), 0.f };
}

glm::mat4 Animator::getFrameMatrix(ModelFrame *frame, float alpha, bool ignoreRoot) const
{
	auto it = _boneMatrices.find( frame );
	if( it != _boneMatrices.end() ) {
		const AnimationKeyframe& S = it->second.first;
		const AnimationKeyframe& F = it->second.second;

		AnimationKeyframe kf {
			glm::slerp(F.rotation, S.rotation, alpha),
			glm::mix(F.position, S.position, alpha),
			glm::mix(F.scale, S.scale, alpha),
			glm::mix(F.starttime, S.starttime, alpha)
		};

		glm::mat4 m;
		bool isRoot = frame->getParent() ? !! frame->getParent() : false;
		if(it->second.bone->type == AnimationBone::R00 || ( isRoot && ignoreRoot ) ) {
			m = glm::translate(m, frame->getDefaultTranslation());
			m = m * glm::mat4_cast(kf.rotation);
		}
		else if(it->second.bone->type == AnimationBone::RT0) {
			m = glm::translate(m, kf.position);
			m = m * glm::mat4_cast(kf.rotation);
		}
		else {
			m = glm::translate(m, kf.position);
			m = m * glm::mat4_cast(kf.rotation);
		}
		return m;
	}
	return frame->getTransform();
}

bool Animator::isCompleted() const
{
	return getAnimation() ? serverTime >= getAnimation()->duration : true;
}

float Animator::getAnimationTime(float alpha) const
{
	if(repeat) {
		return fmod(serverTime + alpha, getAnimation()->duration);
	}
	return serverTime + alpha;
}
