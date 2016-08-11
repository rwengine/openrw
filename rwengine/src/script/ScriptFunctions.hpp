#ifndef RWENGINE_SCRIPTFUNCTIONS_HPP
#define RWENGINE_SCRIPTFUNCTIONS_HPP

#include <engine/GameWorld.hpp>
#include <engine/GameState.hpp>
#include <engine/GameData.hpp>
#include <objects/GameObject.hpp>
#include <script/ScriptTypes.hpp>
#include <script/ScriptMachine.hpp>

/**
 * Implementaitions for common functions likely to be shared
 * among many script modules and opcodes.
 */
namespace script {
const ScriptVec3 kSpawnOffset = ScriptVec3(0.f, 0.f, 1.f);

inline void getObjectPosition(GameObject* object,
                              ScriptFloat& x,
                              ScriptFloat& y,
                              ScriptFloat& z)
{
	const auto& p = object->getPosition();
	x = p.x;
	y = p.y;
	z = p.z;
}
inline void setObjectPosition(GameObject* object, const ScriptVec3& coord)
{
	object->setPosition(coord + script::kSpawnOffset);
}

inline VehicleObject* getCharacterVehicle(CharacterObject* character)
{
	return character->getCurrentVehicle();
}
inline bool isInModel(const ScriptArguments& args,
                      CharacterObject* character,
                      int model)
{
	auto data = args.getWorld()->data->findObjectType<VehicleData>(model);
	if (data) {
		auto vehicle = getCharacterVehicle(character);
		if (vehicle) {
			return vehicle->model ? vehicle->model->name == data->modelName
			                      : false;
		}
	}
	return false;
}

inline bool objectInBounds(GameObject* object,
                           const ScriptVec2& min,
                           const ScriptVec2& max)
{
	const auto& p = object->getPosition();
	return (p.x >= min.x && p.y >= min.y && p.x <= max.x && p.y <= max.y);
}
inline bool objectInBounds(GameObject* object,
                           const ScriptVec3& min,
                           const ScriptVec3& max)
{
	const auto& p = object->getPosition();
	return (p.x >= min.x && p.y >= min.y && p.z >= min.z && p.x <= max.x &&
	        p.y <= max.y && p.z <= max.z);
}
template <class Tobj, class Tvec>
bool objectInArea(const ScriptArguments& args, const Tobj& object, const Tvec& min, const Tvec& max, bool marker)
{
	if (marker) {
		auto center = (min+max);
		auto radius = (max-min);
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(center.x, center.y, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground, glm::vec3(radius.x, radius.y, 5.f));
	}
	return objectInBounds(object, glm::min(min, max), glm::max(min, max));
}

inline bool objectInSphere(GameObject* object,
                           const ScriptVec2& center,
                           const ScriptVec2& radius)
{
	auto p = glm::vec2(object->getPosition());
	p = glm::abs(p - center);
	return p.x <= radius.x && p.y <= radius.y;
}
inline bool objectInSphere(GameObject* object,
                           const ScriptVec3& center,
                           const ScriptVec3& radius)
{
	auto p = object->getPosition();
	p = glm::abs(p - center);
	return p.x <= radius.x && p.y <= radius.y && p.z <= radius.z;
}
template <class Tobj, class Tvec>
inline bool objectInRadius(const ScriptArguments& args, const Tobj& object, const Tvec& center, const Tvec& radius, bool marker)
{
	if (marker) {
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(center.x, center.y, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground, glm::vec3(radius.x, radius.y, 5.f));
	}
	return objectInSphere(object, center, radius);
}
template <class Tvec>
inline bool objectInRadiusNear(const ScriptArguments& args, GameObject* object, GameObject* near, const Tvec& radius, bool marker)
{
	Tvec center (near->getPosition());
	if (marker) {
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(center.x, center.y, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground, glm::vec3(radius.x, radius.y, 5.f));
	}
	return objectInSphere(object, center, radius);
}

template<class Tobj>
inline bool objectInZone(const ScriptArguments& args, Tobj object, const ScriptString zone) {
	auto zfind = args.getWorld()->data->zones.find(zone);
	if( zfind != args.getWorld()->data->zones.end() ) {
		auto& min = zfind->second.min;
		auto& max = zfind->second.max;
		return objectInBounds(object, min, max);
	}
	return false;
}

inline void destroyObject(const ScriptArguments& args, GameObject* object)
{
	args.getWorld()->destroyObjectQueued(object);
}

inline ScriptVec3 getGround(const ScriptArguments& args, ScriptVec3 p)
{
	if (p.z < -90.f) {
		p = args.getWorld()->getGroundAtPosition(p);
	}
	return p;
}

inline std::string gxt(const ScriptArguments& args, const ScriptString id)
{
	return args.getWorld()->data->texts.text(id);
}

inline BlipData createObjectBlip(const ScriptArguments& args, GameObject* object)
{
	BlipData data;
	switch(object->type()) {
	case GameObject::Vehicle:
		data.type = BlipData::Vehicle;
		break;
	case GameObject::Character:
		data.type = BlipData::Character;
		break;
	case GameObject::Pickup:
		data.type = BlipData::Pickup;
		break;
	default:
		data.type = BlipData::Location;
		RW_ERROR("Unhandled blip type");
		break;
	}
	data.target = object->getScriptObjectID();
	data.texture = "";
	args.getState()->addRadarBlip(data);
	return data;
}

const char* getBlipSprite(ScriptRadarSprite sprite);

inline BlipData createObjectBlipSprite(const ScriptArguments& args, GameObject* object, int sprite)
{
	BlipData data;
	args.getState()->addRadarBlip(data);
	switch(object->type()) {
	case GameObject::Vehicle:
		data.type = BlipData::Vehicle;
		break;
	case GameObject::Character:
		data.type = BlipData::Character;
		break;
	case GameObject::Pickup:
		data.type = BlipData::Pickup;
		break;
	default:
		data.type = BlipData::Location;
		RW_ERROR("Unhandled blip type");
		break;
	}
	data.target = object->getScriptObjectID();
	data.texture = getBlipSprite(sprite);
	return data;
}

ScriptModel getModel(const ScriptArguments& args, ScriptModel model);

inline void objectPostCreate(const ScriptArguments& args, GameObject* object)
{
	if (args.getThread()->isMission) {
		/// @todo verify if the mission object list should be kept on a per-thread basis?
		args.getState()->missionObjects.push_back(object);
	}
}

}

#endif
