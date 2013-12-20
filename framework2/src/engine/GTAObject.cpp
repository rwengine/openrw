#include <engine/GTAObject.hpp>
#include <loaders/LoaderIFP.hpp>
#include <loaders/LoaderDFF.hpp>

void GTAObject::setPosition(const glm::vec3& pos)
{
	position = pos;
}

glm::vec3 GTAObject::getPosition() const
{
	return position;
}

glm::quat GTAObject::getRotation() const
{
	return rotation;
}
