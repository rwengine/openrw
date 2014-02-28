#include <engine/GameObject.hpp>
#include <loaders/LoaderIFP.hpp>
#include <loaders/LoaderDFF.hpp>

void GameObject::setPosition(const glm::vec3& pos)
{
	position = pos;
}

glm::vec3 GameObject::getPosition() const
{
	return position;
}

glm::quat GameObject::getRotation() const
{
	return rotation;
}
