#ifndef RWENGINE_SCRIPTFUNCTIONS_HPP
#define RWENGINE_SCRIPTFUNCTIONS_HPP

#include <engine/GameWorld.hpp>
#include <engine/GameState.hpp>
#include <engine/GameData.hpp>
#include <objects/GameObject.hpp>
#include <script/ScriptTypes.hpp>

/**
 * Implementaitions for common functions likely to be shared
 * among many script modules and opcodes.
 */
namespace script {
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
	object->setPosition(coord);
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
bool objectInArea(const ScriptArguments& args, Tobj* object, const Tvec& min, const Tvec& max, bool marker)
{
	if (marker) {
		RW_UNUSED(args);
		RW_UNIMPLEMENTED("Area check marker");
	}
	return objectInBounds(object, min, max);
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
	data.texture = "";
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
}

#endif
