#include <engine/GameObject.hpp>
#include <loaders/LoaderIFP.hpp>
#include <loaders/LoaderDFF.hpp>
#include <engine/Animator.hpp>
#include <data/Skeleton.hpp>

GameObject::~GameObject()
{
	if(animator)
	{
		delete animator;
	}
	if(skeleton)
	{
		delete skeleton;
	}
}


void GameObject::setPosition(const glm::vec3& pos)
{
	_lastPosition = position = pos;
}

glm::quat GameObject::getRotation() const
{
	return rotation;
}

void GameObject::setRotation(const glm::quat& orientation)
{
	rotation = orientation;
}

void GameObject::setHeading(float heading)
{
	auto hdg = (heading / 180.f) * glm::pi<float>();
	auto quat = glm::normalize(glm::quat(glm::vec3(0.f, 0.f, hdg)));
	setRotation(quat);
}
