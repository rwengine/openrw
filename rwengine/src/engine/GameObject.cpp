#include <engine/GameObject.hpp>
#include <loaders/LoaderIFP.hpp>
#include <loaders/LoaderDFF.hpp>

void GameObject::setPosition(const glm::vec3& pos)
{
	_lastPosition = position = pos;
}

glm::quat GameObject::getRotation() const
{
	return rotation;
}
