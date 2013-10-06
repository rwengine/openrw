#include <renderwure/engine/GTAObject.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/engine/GTAEngine.hpp>

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
