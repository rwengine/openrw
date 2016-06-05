#include <script/modules/ObjectModule.hpp>

#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <data/Model.hpp>
#include <render/GameRenderer.hpp>
#include <engine/Animator.hpp>
#include <engine/GameState.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/ItemPickup.hpp>
#include <core/Logger.hpp>
#include <items/InventoryItem.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

glm::vec3 spawnMagic( 0.f, 0.f, 1.f );

void game_create_player(const ScriptArguments& args)
{
	auto id = args[0].integer;
	if (id != 0) {
		RW_UNIMPLEMENTED("Multiple player characters not supported");
	}

	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	
	if( position.z < -99.f ) {
		position = args.getWorld()->getGroundAtPosition(position);
	}
	
	auto pc = args.getWorld()->createPlayer(position + spawnMagic);
    args.getState()->playerObject = pc->getGameObjectID();
    *args[4].globalInteger = args.getWorld()->players.size()-1;
}

template<class Tobject>
void game_set_object_position(const ScriptArguments& args)
{
	auto character = args.getObject<Tobject>(0);
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	character->setPosition(position + spawnMagic);
}

bool game_player_in_area_2d(const ScriptArguments& args)
{
    auto character = args.getPlayerCharacter(0);
	glm::vec2 min(args[1].real, args[2].real);
	glm::vec2 max(args[3].real, args[4].real);
	auto player = character->getPosition();
	if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
		return true;
	}
	return false;
}

bool game_player_in_area_3d(const ScriptArguments& args)
{
    auto character = args.getPlayerCharacter(0);
	glm::vec3 min(args[1].real, args[2].real, args[3].real);
	glm::vec3 max(args[4].real, args[5].real, args[6].real);
	auto player = character->getPosition();
	if( player.x > min.x &&
		player.y > min.y &&
		player.z > min.z &&
		player.x < max.x &&
		player.y < max.y &&
		player.z < max.z) {
		return true;
	}
	return false;
}

void game_create_character(const ScriptArguments& args)
{
	auto type = args[0].integer;
	auto id	= args[1].integer;
	glm::vec3 position(args[2].real, args[3].real, args[4].real);
	
	if( type == 21 ) {
		
	}
	if( position.z < -99.f ) {
		position = args.getWorld()->getGroundAtPosition(position);
	}
	
	auto character = args.getWorld()->createPedestrian(id, position + spawnMagic);
	/* Controller will give ownership to character */
	new DefaultAIController(character);
	
	if ( args.getThread()->isMission )
	{
		args.getState()->missionObjects.push_back(character->getGameObjectID());
	}
	
	*args[5].globalInteger = character->getGameObjectID();
}

void game_create_vehicle(const ScriptArguments& args)
{
	auto id	= args[0].integer;
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	if( position.z < -99.f ) {
		position = args.getWorld()->getGroundAtPosition(position);
	}
	position += spawnMagic;

	auto vehicle = args.getWorld()->createVehicle(id, position);

	if ( args.getThread()->isMission )
	{
		args.getState()->missionObjects.push_back(vehicle->getGameObjectID());
	}

	*args[4].globalInteger = vehicle->getGameObjectID();
}

void game_get_vehicle_position(const ScriptArguments& args)
{
	auto vehicle = args.getObject<VehicleObject>(0);
	
	if( vehicle )
	{
		auto vp = vehicle->getPosition();
		*args[1].globalReal = vp.x;
		*args[2].globalReal = vp.y;
		*args[3].globalReal = vp.z;
	}
}

template <class T>
void game_get_character_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	*args[1].globalInteger = character->getCurrentVehicle()->getGameObjectID();
}

template <class T>
bool game_character_in_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	auto vehicle = args.getObject<VehicleObject>(1);
	
	if( character == nullptr || vehicle == nullptr )
	{
		return false;
	}
	
	return character->getCurrentVehicle() == vehicle;
}

bool game_character_in_model(const ScriptArguments& args)
{
	auto vdata = args.getWorld()->data->findObjectType<VehicleData>(args[1].integer);
	if( vdata )
	{
		auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
		auto vehicle = character->getCurrentVehicle();
		if ( vehicle ) {
			
			return vehicle->model && vdata->modelName == vehicle->model->name;
		}
	}
	return false;
}

bool game_player_in_model(const ScriptArguments& args)
{
    auto vdata = args.getWorld()->data->findObjectType<VehicleData>(args[1].integer);
    if( vdata )
    {
        auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
        auto vehicle = character->getCurrentVehicle();
        if ( vehicle ) {

            return vehicle->model && vdata->modelName == vehicle->model->name;
        }
    }
    return false;
}


bool game_character_in_any_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	
	auto vehicle = character->getCurrentVehicle();
	return vehicle != nullptr;
}

bool game_player_in_any_vehicle(const ScriptArguments& args)
{
    auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));

    auto vehicle = character->getCurrentVehicle();
    return vehicle != nullptr;
}

bool game_player_in_area_2d_in_vehicle(const ScriptArguments& args)
{
    auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	
	bool drawCylinder = args[5].integer;
	
	if( character->getCurrentVehicle() == nullptr )
	{
		return false;
	}
	
	auto vp = character->getCurrentVehicle()->getPosition();
	glm::vec2 distance = glm::abs(position - glm::vec2(vp));
	
	if(distance.x <= radius.x && distance.y <= radius.y)
	{
		return true;
	}
	
	if( drawCylinder )
	{
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(position, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground + glm::vec3(0.f, 0.f, 4.5f), glm::vec3(radius, 5.f));
	}
	
	return false;
}

bool game_character_near_point_on_foot_3D(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool drawCylinder = !!args[7].integer;
	
	auto vehicle = character->getCurrentVehicle();
	if( ! vehicle ) {
		auto distance = center - character->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	if( drawCylinder )
	{
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, center, size);
	}
	
	return false;
}

bool game_player_near_point_on_foot_3D(const ScriptArguments& args)
{
    auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
    glm::vec3 center(args[1].real, args[2].real, args[3].real);
    glm::vec3 size(args[4].real, args[5].real, args[6].real);
    bool drawCylinder = !!args[7].integer;

    auto vehicle = character->getCurrentVehicle();
    if( ! vehicle ) {
        auto distance = center - character->getPosition();
        distance /= size;
        if( glm::length( distance ) < 1.f ) return true;
    }

    if( drawCylinder )
    {
        args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, center, size);
    }

    return false;
}

bool game_player_near_point_in_vehicle_3D(const ScriptArguments& args)
{
    auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
    glm::vec3 center(args[1].real, args[2].real, args[3].real);
    glm::vec3 size(args[4].real, args[5].real, args[6].real);
    bool unkown	= !!args[7].integer;
	RW_UNUSED(unkown);

    auto vehicle = character->getCurrentVehicle();
    if( vehicle ) {
        auto distance = center - character->getPosition();
        distance /= size;
        if( glm::length( distance ) < 1.f ) return true;
    }

    return false;
}

bool game_player_stopped_near_point_on_foot_3d(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool drawCylinder = !!args[7].integer;

	auto vehicle = character->getCurrentVehicle();
	if( ! vehicle ) {
		auto distance = center - character->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f && character->isStopped() ) {
			return true;
		}
	}

	if( drawCylinder )
	{
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, center, size);
	}

	return false;
}

bool game_character_near_point_in_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool unkown	= !!args[7].integer;
	RW_UNUSED(unkown);

	auto vehicle = character->getCurrentVehicle();
	if( vehicle ) {
		auto distance = center - character->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

template <class T>
bool game_locate_character_stopped_2d(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	
	bool drawCylinder = args[5].integer;
	
	auto cp = character->getPosition();
	glm::vec2 distance = glm::abs(position - glm::vec2(cp));
	
	if(distance.x <= radius.x && distance.y <= radius.y && character->isStopped())
	{
		return true;
	}
	
	if( drawCylinder )
	{
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(position, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground, glm::vec3(radius, 5.f));
	}
	
	return false;
}

bool game_locate_player_stopped_in_vehicle_2d(const ScriptArguments& args)
{
  auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	
	bool drawCylinder = args[5].integer;
	
	if( character->getCurrentVehicle() == nullptr )
	{
		return false;
	}
	
	auto vp = character->getCurrentVehicle()->getPosition();
	glm::vec2 distance = glm::abs(position - glm::vec2(vp));
	
	if(distance.x <= radius.x && distance.y <= radius.y && character->isStopped())
	{
		return true;
	}
	
	if( drawCylinder )
	{
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(position, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground, glm::vec3(radius, 5.f));
	}
	
	return false;
}

bool game_character_near_character_2D(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	RW_CHECK(character != nullptr, "character is null");
	auto target = args.getObject<CharacterObject>(1);
	RW_CHECK(target != nullptr, "target is null");
	if (character == nullptr || target == nullptr) return false;

	glm::vec2 center(target->getPosition());
	glm::vec2 size(args[2].real, args[3].real);
	bool unkown = !!args[4].integer;
	RW_UNUSED(unkown);

	auto distance = center - glm::vec2(character->getPosition());
	distance /= size;
	return glm::length( distance ) < 1.f;
	
	return false;
}

template <class T>
bool game_character_near_character_in_vehicle_2D(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	auto target = args.getObject<CharacterObject>(1);
	glm::vec2 center(target->getPosition());
	glm::vec2 size(args[2].real, args[3].real);
	bool unkown = !!args[4].integer;
	RW_UNUSED(unkown);

	auto vehicle = character->getCurrentVehicle();
	if( vehicle ) {
		auto distance = center - glm::vec2(character->getPosition());
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_near_point_on_foot_2D(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	glm::vec2 center(args[1].real, args[2].real);
	glm::vec2 size(args[3].real, args[4].real);
	bool unkown = !!args[5].integer;
	RW_UNUSED(unkown);

	auto vehicle = character->getCurrentVehicle();
	if( !vehicle ) {
		auto distance = center - glm::vec2(character->getPosition());
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_dead(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	
	if ( character )
	{
		return !character->isAlive();
	}
	return true;
}

bool game_vehicle_dead(const ScriptArguments& args)
{
	// TODO This won't work until vehicle destruction is finished
	auto vehicle = args.getObject<VehicleObject>(0);
	return vehicle == nullptr;
}

template <class T>
bool game_character_in_zone(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	std::string zname(args[1].string);
	
	auto zfind = args.getWorld()->data->zones.find(zname);
	if( zfind != args.getWorld()->data->zones.end() ) {
		auto player = character->getPosition();
		auto& min = zfind->second.min;
		auto& max = zfind->second.max;
		if( player.x > min.x && player.y > min.y && player.z > min.z &&
			player.x < max.x && player.y < max.y && player.z < max.z ) {
			return true;
			}
	}
	
	return false;
}

bool game_player_pressing_horn(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	if ( character->getCurrentVehicle() != nullptr )
	{
		/// @todo Respect actual horn key.
		return true;
	}

	return false;
}

bool game_character_objective_passed(const ScriptArguments& args)
{
  auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	if( character && character->controller->getCurrentActivity() == nullptr )
	{
		return true;
	}

	return false;
}

void game_create_character_in_vehicle(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	auto type = args[1].integer;
	RW_UNUSED(type);
	RW_UNIMPLEMENTED("game_create_character_in_vehicle(): character type");
	auto id = args[2].integer;
	
	auto character = args.getWorld()->createPedestrian(id, vehicle->getPosition() + spawnMagic);
	new DefaultAIController(character);
	
	character->setCurrentVehicle(vehicle, 0);
	vehicle->setOccupant(0, character);
	
	*args[3].globalInteger = character->getGameObjectID();
}

bool game_is_vehicle_model(const ScriptArguments& args)
{
    auto vdata = args.getWorld()->data->findObjectType<VehicleData>(args[1].integer);
    if( vdata )
    {
        auto vehicle = args.getObject<VehicleObject>(0);
        if ( vehicle ) {

            return vehicle->model && vdata->modelName == vehicle->model->name;
        }
    }
    return false;
}


void game_set_player_heading(const ScriptArguments& args)
{
    auto object = args.getPlayerCharacter(0);
	object->setHeading(args[1].real);
}

template <class Tobject>
void game_get_object_heading(const ScriptArguments& args)
{
	auto object = args.getObject<Tobject>(0);
	*args[1].globalReal = object->getHeading();
}

template <class Tobject>
void game_set_object_heading(const ScriptArguments& args)
{
	auto object = args.getObject<Tobject>(0);
	object->setHeading(args[1].real);
}

void game_give_weapon_to_character(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	CharacterState& cs = character->getCurrentState();
	int weaponId = args[1].integerValue();
	int bulletsTotal = args[2].integerValue();

	RW_CHECK(weaponId >= 0, "Weapon-ID too low");
	RW_CHECK(weaponId < static_cast<int>(cs.weapons.size()), "Weapon-ID too high");

	// Give character the weapon
	auto& weapon = cs.weapons[weaponId];
	weapon.weaponId = weaponId;
	weapon.bulletsClip = bulletsTotal; /// @todo what to set here?
	weapon.bulletsTotal = bulletsTotal;

	// Set active weapon
	cs.currentWeapon = weaponId;
}

bool game_vehicle_stopped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	
	if( vehicle )
	{
		return std::abs( vehicle->physVehicle->getCurrentSpeedKmHour() ) <= 0.01f;
	}
	return false;
}

/// Remove object from cleanup at end of missions.
void game_dont_remove_object(const ScriptArguments& args)
{
    auto object = args.getObject<VehicleObject>(0);
    if(object)
    {
        auto id = object->getGameObjectID();
	
        auto& mO = args.getState()->missionObjects;
        mO.erase(std::remove(mO.begin(), mO.end(), id), mO.end());
    }
}

template <class T>
bool game_character_in_area_or_cylinder(const ScriptArguments& args)
{
	auto character = args.getPlayerCharacter(0);
	glm::vec2 min(args[1].real, args[2].real);
	glm::vec2 max(args[3].real, args[4].real);
	bool isCylinder = !!args[5].integerValue();

	RW_UNIMPLEMENTED("game_character_in_area_or_cylinder: should use cylinder if requested?");
	RW_UNUSED(isCylinder);
	/// @todo

	auto player = character->getPosition();
	if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
		return true;
	}
	return false;
}

template <class T>
bool game_character_in_area_on_foot(const ScriptArguments& args)
{
	RW_UNIMPLEMENTED("game_character_in_area_on_foot");
	RW_UNUSED(args);
	/// @todo
	return false;
}

template <class T>
bool game_character_stoped_in_volume_in_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	bool drawCylinder = !!args[7].integer;
	
	if( character && character->getCurrentVehicle() != nullptr )
	{
		glm::vec3 vec1(args[1].real, args[2].real, args[3].real);
		glm::vec3 vec2(args[4].real, args[5].real, args[6].real);
		glm::vec3 min = glm::min(vec1, vec2);
		glm::vec3 max = glm::max(vec1, vec2);
		
		glm::vec3 pp = character->getCurrentVehicle()->getPosition();
		
		if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
			pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
		{
			return character->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
		}
		
		// Request the renderer draw a cylinder here.
		if( drawCylinder )
		{
			args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, (max+min)/2.f, (max-min)/2.f);
		}
	}
	return false;
}


template <class T>
bool game_character_stoped_in_volume(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	
	glm::vec3 vec1(args[1].real, args[2].real, args[3].real);
	glm::vec3 vec2(args[4].real, args[5].real, args[6].real);
	bool drawCylinder = !!args[7].integer;
	
	glm::vec3 min = glm::min(vec1, vec2);
	glm::vec3 max = glm::max(vec1, vec2);
	
	glm::vec3 pp = character->getPosition();
	
	if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
		pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
	{
		if( character->getCurrentVehicle() != nullptr )
		{
			return character->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
		}
		else
		{
			return character->controller->getCurrentActivity() == nullptr;
		}
	}
	
	if( drawCylinder )
	{
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, (max+min)/2.f, (max-min)/2.f);
	}
	
	return false;
}

bool game_locate_vehicle_near_point_2D(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	bool drawCylinder = !!args[5].integerValue();

	auto vp = vehicle->getPosition();
	glm::vec2 distance = glm::abs(position - glm::vec2(vp));

	if(distance.x <= radius.x && distance.y <= radius.y) {
		return true;
	}

	if(drawCylinder) {
		auto ground = args.getWorld()->getGroundAtPosition(glm::vec3(position, 100.f));
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, ground + glm::vec3(0.f, 0.f, 4.5f), glm::vec3(radius, 5.f));
	}

	return false;
}

template <class T>
bool game_is_character_stopped(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<T>(0));
	
	if( character && character->getCurrentVehicle() != nullptr )
	{
		return character->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
	}
	
	return true;
}


template <class T>
bool game_character_in_area_9(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_character_in_area_9");
	return false;
}

bool game_objects_in_volume(const ScriptArguments& args)
{
	glm::vec3 vec1(args[0].real, args[1].real, args[2].real);
	glm::vec3 vec2(args[3].real, args[4].real, args[5].real);
	glm::vec3 min = glm::min(vec1, vec2);
	glm::vec3 max = glm::max(vec1, vec2);
	
	bool solids = args[6].integer;
	bool cars = args[7].integer;
	bool actors = args[8].integer;
	bool objects = args[9].integer;
	bool particles = args[10].integer;
	RW_UNUSED(objects);
	RW_UNUSED(particles);
	
	for(auto& object : args.getWorld()->allObjects)
	{
		switch( object->type() )
		{
			case GameObject::Instance:
				if( !solids ) continue;
				break;
			case GameObject::Character:
				if( !actors ) continue;
				break;
			case GameObject::Vehicle:
				if( !cars ) continue;
				break;
			default:
				continue;
		}
		
		// Maybe consider object bounds?
		auto pp = object->getPosition();
		
		if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
			pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
		{
			return true;
		}
	}
	return false;
}

bool game_player_in_taxi(const ScriptArguments& args)
{
    auto character = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	
	auto vehicle = character->getCurrentVehicle();
	return (vehicle && (vehicle->vehicle->classType & VehicleData::TAXI) == VehicleData::TAXI);
}

void game_get_speed(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	if( vehicle )
	{
		*args[1].globalReal = vehicle->physVehicle->getCurrentSpeedKmHour();
	}
}

void game_enter_as_driver(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(1));
	// Cancel whatever we're currently trying to do.
	character->controller->skipActivity();
	character->controller->setNextActivity(new Activities::EnterVehicle(vehicle,0));
}

void game_enter_as_passenger(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(1));
	
	// Cancel whatever we're currently trying to do.
	character->controller->skipActivity();
	/// @todo find next lowest free seat.
	character->controller->setNextActivity(new Activities::EnterVehicle(vehicle,Activities::EnterVehicle::ANY_SEAT));
}

void game_character_exit_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(1));
	auto cvehcile = character->getCurrentVehicle();
	
	if( cvehcile && cvehcile == vehicle )
	{
		character->controller->setNextActivity(new Activities::ExitVehicle());
	}
}

void game_character_follow_character(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	auto leader = static_cast<CharacterObject*>(args.getObject<CharacterObject>(1));
	
	character->controller->setGoal(CharacterController::FollowLeader);
	character->controller->setTargetCharacter(leader);
}

void game_navigate_on_foot(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	glm::vec3 target(args[1].real, args[2].real, 0.f);
	target = args.getWorld()->getGroundAtPosition(target);
	
	character->controller->skipActivity();
	
	if( character->getCurrentVehicle() )
	{
		// Since we just cleared the Activities, this will become current immediatley.
		character->controller->setNextActivity(new Activities::ExitVehicle);
	}
	
	character->controller->setNextActivity(new Activities::GoTo(target));
}

void game_create_pickup(const ScriptArguments& args)
{
	glm::vec3 pos (args[2].real, args[3].real, args[4].real);
	int id;
	int type = args[1].integer;
	
	switch(args[0].type) {
		case TInt8:
			id = (std::int8_t)args[0].integer;
			break;
		case TInt16:
			id = (std::int16_t)args[0].integer;
			break;
		default:
			RW_ERROR("Unhandled integer type");
			*args[5].globalInteger = 0;
			return;
	}
	
	if ( id < 0 )
	{
		id = -id;
		
		auto model = args.getVM()->getFile()->getModels()[id];
		std::transform(model.begin(), model.end(), model.begin(), ::tolower);
	
		id = args.getWorld()->data->findModelObject(model);
		args.getWorld()->data->loadDFF(model+".dff");
		args.getWorld()->data->loadTXD("icons.txd");
	}
	else
	{
		auto data = args.getWorld()->data->findObjectType<ObjectData>(id);
		
		if ( ! ( id >= 170 && id <= 184 ) )
		{
			args.getWorld()->data->loadDFF(data->modelName+".dff");
		}
		args.getWorld()->data->loadTXD(data->textureName+".txd");
	}
	
	PickupObject* pickup = nullptr;

	if ( id >= 170 && id <= 184 )
	{
		// Find the item for this model ID
		auto world = args.getWorld();
		InventoryItem *item = nullptr;
		for (auto i = 0; i < maxInventorySlots; ++i)
		{
			item = world->getInventoryItem(i);
			if (item->getModelID() == id) {
				auto pickuptype = (PickupObject::PickupType)type;
				pickup = new ItemPickup(args.getWorld(), pos, pickuptype, item);
				world->pickupPool.insert( pickup );
				world->allObjects.push_back(pickup);
				*args[5].globalInteger = pickup->getGameObjectID();
			}
		}
	}
	else
	{
		RW_UNIMPLEMENTED("non-item pickups");
		*args[5].globalInteger = 0;
	}
	
}

bool game_is_pickup_collected(const ScriptArguments& args)
{
	PickupObject* pickup = static_cast<PickupObject*>(args.getObject<PickupObject>(0));
	
	if ( pickup )
	{
		return pickup->isCollected();
	}
	
	return false;
}

void game_destroy_pickup(const ScriptArguments& args)
{
	PickupObject* pickup = static_cast<PickupObject*>(args.getObject<PickupObject>(0));
	
	if ( pickup )
	{
		args.getWorld()->destroyObjectQueued(pickup);
	}
}

void game_character_run_to(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	glm::vec3 target(args[1].real, args[2].real, 0.f);
	target = args.getWorld()->getGroundAtPosition(target);
	
	character->controller->setNextActivity(new Activities::GoTo(target, true));
}

bool game_vehicle_flipped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	
	if( vehicle )
	{
		return vehicle->isFlipped();
	}
	
	return false;
}

bool game_vehicle_in_air(const ScriptArguments& args)
{
	/// @todo IS vehicle in air.
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	RW_UNUSED(vehicle);
	RW_UNIMPLEMENTED("game_vehicle_in_air()");
	return false;
}

bool game_character_near_car_2d(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(1));
	glm::vec2 radius(args[2].real, args[3].real);
	bool drawMarker = !!args[4].integer;
	RW_UNUSED(drawMarker);
	RW_UNIMPLEMENTED("Draw marker in game_character_near_car_2D");

	auto charVehicle = character->getCurrentVehicle();
	if( charVehicle ) {
		auto dist = charVehicle->getPosition() - vehicle->getPosition();
		if( dist.x <= radius.x && dist.y <= radius.y ) {
			return true;
		}
	}
	
	return false;
}

void game_set_vehicle_colours(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	
	auto& colours = args.getWorld()->data->vehicleColours;
	vehicle->colourPrimary = colours[args[1].integer];
	vehicle->colourSecondary = colours[args[2].integer];
}

void game_create_object_world(const ScriptArguments& args)
{
	int id = 0;
	switch(args[0].type) {
		case TInt8:
			id = (std::int8_t)args[0].integer;
			break;
		case TInt16:
			id = (std::int16_t)args[0].integer;
			break;
		default:
			RW_ERROR("Unhandled integer type");
			break;
	}
	
	if( id < 0 ) {
		auto& modelname = args.getVM()->getFile()->getModels()[-id];
		id = args.getWorld()->data->findModelObject(modelname);
		if( id == (uint16_t)-1 ) {
			args.getWorld()->logger->error("SCM", "Failed to find model " + modelname);
		}
	}

	glm::vec3 position(args[1].real, args[2].real, args[3].real);

	auto inst = args.getWorld()->createInstance(id, position);

	*args[4].globalInteger = inst->getGameObjectID();
}

template <class Tobject>
void game_destroy_object(const ScriptArguments& args)
{
	auto object = args.getObject<Tobject>(0);
	RW_CHECK(object != nullptr, "destroying a null object?");
	
	args.getWorld()->destroyObjectQueued(object);
}

bool game_is_boat(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_is_boat()");
	/*auto vehicle = (VehicleObject*)(*args[0].handle);
	 *	if( vehicle )
	 *	{
	 *		return vehicle->vehicle->type == VehicleData::BOAT;
}*/
	return false;
}

bool game_character_in_range(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_character_in_range()");
	return true;
}

void game_set_close_object_visible(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	float radius = args[3].real;
	int modelid = 0;
	
	/// @todo fix this being a problem.
	switch(args[4].type) {
		case TInt8:
			modelid = (std::int8_t)args[4].integer;
			break;
		case TInt16:
			modelid = (std::int16_t)args[4].integer;
			break;
		default:
			RW_ERROR("Unhandled integer type");
			break;
	}
	
	if( std::abs(modelid) > 178 ) {
		/// @todo implement this path,
		return;
	}
	
	std::string model;
	
	if(modelid < 0) modelid = -modelid;
	
	model = args.getVM()->getFile()->getModels()[modelid];
	
	std::transform(model.begin(), model.end(), model.begin(), ::tolower);
	
	for(auto& p : args.getWorld()->instancePool.objects) {
		auto o = p.second;
		if( !o->model ) continue;
		if( o->model->name != model ) continue;
		float d = glm::distance(position, o->getPosition());
		if( d < radius ) {
			o->visible = !!args[5].integer;
		}
	}
}

void game_change_nearest_model(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	float radius = args[3].real;
	int newid = 0, oldid = 0;
	
	/// @todo fix this being a problem.
	switch(args[4].type) {
		case TInt8:
			oldid = (std::int8_t)args[4].integer;
			break;
		case TInt16:
			oldid = (std::int16_t)args[4].integer;
			break;
		default:
			RW_ERROR("Unhandled integer type");
			break;
	}
	
	switch(args[5].type) {
		case TInt8:
			newid = (std::int8_t)args[5].integer;
			break;
		case TInt16:
			newid = (std::int16_t)args[5].integer;
			break;
		default:
			RW_ERROR("Unhandled integer type");
			break;
	}
	
	if( std::abs(newid) > 178 || std::abs(oldid) > 178 ) {
		/// @todo implement this path,
		return;
	}
	
	std::string newmodel;
	std::string oldmodel;
	
	if(newid < 0) newid = -newid;
	if(oldid < 0) oldid = -oldid;
	
	newmodel = args.getVM()->getFile()->getModels()[newid];
	oldmodel = args.getVM()->getFile()->getModels()[oldid];
	std::transform(newmodel.begin(), newmodel.end(), newmodel.begin(), ::tolower);
	std::transform(oldmodel.begin(), oldmodel.end(), oldmodel.begin(), ::tolower);
	
	auto newobjectid = args.getWorld()->data->findModelObject(newmodel);
	auto nobj = args.getWorld()->data->findObjectType<ObjectData>(newobjectid);
	
	/// @todo Objects need to adopt the new object ID, not just the model.
	for(auto p : args.getWorld()->instancePool.objects) {
		auto o = p.second;
		if( !o->model ) continue;
		if( o->model->name != oldmodel ) continue;
		float d = glm::distance(position, o->getPosition());
		if( d < radius ) {
			args.getWorld()->data->loadDFF(newmodel + ".dff", false);
			InstanceObject* inst = static_cast<InstanceObject*>(o);
			inst->changeModel(nobj);
			inst->model = args.getWorld()->data->models[newmodel];
		}
	}
}

bool game_rotate_object(const ScriptArguments& args)
{
	auto object = args.getObject<InstanceObject>(0);
	if( object )
	{
		float start = args[2].real;
		float finish = args[1].real;
		RW_UNUSED(start);

		// @todo INTERPOLATE instead of just setting the heading.
		object->setHeading(finish);
	}
	
	return true;
}

template<class Tobject>
void game_destroy_object_with_fade(const ScriptArguments& args)
{
	/// @todo Fade not handled yet!
	game_destroy_object<Tobject>(args);
}

void game_get_vehicle_colours(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(0));
	
	if ( vehicle )
	{
		/// @TODO use correct values.
		*args[1].globalInteger = 0;
		*args[2].globalInteger = 0;
	}
}


ObjectModule::ObjectModule()
: ScriptModule("Object")
{
	bindFunction(0x0053, game_create_player, 5, "Create Player" );
	
    bindFunction(0x0055, game_set_object_position<PlayerController>, 4, "Set Player Position" );
	bindFunction(0x0056, game_player_in_area_2d, 6, "Is Player In Area 2D" );
	bindFunction(0x0057, game_player_in_area_3d, 8, "Is Player In Area 3D" );
	
	bindFunction(0x009A, game_create_character, 6, "Create Character" );
	bindFunction(0x009B, game_destroy_object<CharacterObject>, 1, "Destroy Character" );
	bindUnimplemented( 0x009C, game_set_character_wander_path, 2, "Set Character Wander Path" );
	
	bindUnimplemented( 0x009F, game_character_make_idle, 1, "Set Character to Idle" );
	
	bindFunction(0x00A1, game_set_object_position<CharacterObject>, 4, "Set Character Position" );
	
	bindFunction(0x00A5, game_create_vehicle, 5, "Create Vehicle" );
	bindFunction(0x00A6, game_destroy_object<VehicleObject>, 1, "Destroy Vehicle" );
	
	bindFunction(0x00AA, game_get_vehicle_position, 4, "Get Vehicle Position" );
	
	bindFunction(0x00D9, game_get_character_vehicle<CharacterObject>, 2, "Get Character Vehicle" );
	bindFunction(0x00DA, game_get_character_vehicle<PlayerController>, 2, "Get Player Vehicle" );
	bindFunction(0x00DB, game_character_in_vehicle<CharacterObject>, 2, "Is Character in Vehicle" );
	bindFunction(0x00DC, game_character_in_vehicle<PlayerController>, 2, "Is Player in Vehicle" );
	
    bindFunction(0x00DE, game_player_in_model, 2, "Is Player In Model" );
    bindFunction(0x00DF, game_character_in_any_vehicle, 1, "Is Character In Any Vehicle" );
    bindFunction(0x00E0, game_player_in_any_vehicle, 1, "Is Player In Any Vehicle" );
	
	bindFunction(0x00E5, game_player_in_area_2d_in_vehicle, 6, "Is Player in 2D Area in Vehicle" );
	
	bindUnimplemented( 0x00E4, game_locate_character_on_foot_2d, 6, "Locate Player on foot 2D" );

	bindFunction(0x00E6, game_locate_character_stopped_2d<PlayerController>, 6, "Locate Player Stopped 2D" );
	
	bindFunction(0x00E8, game_locate_player_stopped_in_vehicle_2d, 6, "Locate Player Stopped in Vehicle 2D" );
	bindFunction(0x00E9, game_character_near_character_2D, 5, "Locate Character near Character 2D");
	
	bindFunction(0x00EB, game_character_near_character_in_vehicle_2D<PlayerController>, 5, "Is player near character in vehicle" );
	
	bindFunction(0x00ED, game_character_near_point_on_foot_2D, 6, "Is Character near point on foot" );
	
	bindFunction(0x00EF, game_locate_character_stopped_2d<CharacterObject>, 6, "Locate Character Stopped 2D" );

	bindUnimplemented( 0x00F5, game_locate_character_in_sphere, 8, "Locate Player In Sphere" );
    bindFunction(0x00F6, game_player_near_point_on_foot_3D, 8, "Is Player near point on foot" );
    bindFunction(0x00F7, game_player_near_point_in_vehicle_3D, 8, "Is Player near point in car" );
	bindFunction(0x00F9, game_player_stopped_near_point_on_foot_3d, 8, "Detect player stopped on foot area 3d" );

	bindFunction(0x0100, game_character_near_point_in_vehicle, 8, "Is Character near point in car" );
	
	bindFunction(0x0108, game_destroy_object<InstanceObject>, 1, "Destroy Object" );
	
	bindFunction(0x0118, game_character_dead, 1, "Is Character Dead" );
	bindFunction(0x0119, game_vehicle_dead, 1, "Is Vehicle Dead" );
	bindUnimplemented(0x011A, game_set_character_search_threat, 2, "Set Character Search Threat" );
	
	bindUnimplemented(0x011C, game_character_clear_objective, 1, "Clear Character Objective" );
	
	bindFunction(0x0121, game_character_in_zone<PlayerController>, 2, "Is Player In Zone" );
    bindFunction(0x0122, game_player_pressing_horn, 1, "Is Player Pressing Horn" );

    bindFunction(0x0126, game_character_objective_passed, 1, "Character Objective Passed" );
	
	bindFunction(0x0129, game_create_character_in_vehicle, 4, "Create Character In Car" );

	bindUnimplemented(0x0135, game_set_vehicle_locked, 2, "Set Vehicle locked state");

    bindFunction(0x0137, game_is_vehicle_model, 2, "Is Vehicle Model" );
	
	bindFunction(0x0171, game_set_player_heading, 2, "Set Player Heading" );
	
	bindFunction(0x0173, game_set_object_heading<CharacterObject>, 2, "Set Character Heading" );
	bindUnimplemented(0x0174, game_get_character_heading, 2, "Get Vehicle Heading" );
	bindFunction(0x0175, game_set_object_heading<VehicleObject>, 2, "Set Vehicle heading" );
	
	bindFunction(0x0176, game_get_object_heading<InstanceObject>, 2, "Get Object heading" );
	bindFunction(0x0177, game_set_object_heading<InstanceObject>, 2, "Set Object heading" );
	
	bindUnimplemented( 0x0192, game_character_stand_still, 1, "Make character stand still" );
	
	bindFunction(0x0199, game_character_in_area_or_cylinder<PlayerController>, 6, "Is Player in Area or Cylinder" );

	bindFunction(0x019C, game_character_in_area_on_foot<PlayerController>, 8, "Is Player in Area on Foot" );
	bindFunction(0x019E, game_character_stoped_in_volume<PlayerController>, 8, "Is Player stopped in volume" );
	
	bindFunction(0x01A0, game_character_stoped_in_volume_in_vehicle<PlayerController>, 8, "Is Player Stopped in cube in vehicle" );
	bindFunction(0x01A8, game_character_stoped_in_volume<CharacterObject>, 8, "Is Char Stopped in volume" );
	bindFunction(0x01AA, game_character_stoped_in_volume_in_vehicle<CharacterObject>, 8, "Is Char Stopped in cube in vehicle" );

	bindFunction(0x01AD, game_locate_vehicle_near_point_2D, 6, "Locate Vehicle Near Point 2D" );

	bindFunction(0x01B2, game_give_weapon_to_character, 3, "Give Weapon to Character" );

	bindUnimplemented( 0x01BB, game_object_coordinates, 4, "Get Object Coordinates" );
	
	bindUnimplemented( 0x01BE, game_turn_character, 4, "Turn Character To Face Point" );
	
	bindFunction(0x01C1, game_vehicle_stopped, 1, "Is Vehicle Stopped" );
	bindUnimplemented( 0x01C2, game_mark_object_as_unneeded<CharacterObject>, 1, "Mark Character Unneeded" );
	bindUnimplemented( 0x01C3, game_mark_object_as_unneeded<VehicleObject>, 1, "Mark Vehicle Unneeded" );
	
	bindFunction(0x01C7, game_dont_remove_object, 1, "Don't remove object" );
	
	bindFunction(0x01D5, game_enter_as_driver, 2, "Character Enter Vehicle as Driver" );
	bindFunction(0x01D4, game_enter_as_passenger, 2, "Character Enter Vehicle as Passenger" );
	bindFunction(0x01D3, game_character_exit_vehicle, 2, "Character Exit Vehicle" );
	
	bindFunction(0x01DF, game_character_follow_character, 2, "Make Character Follow Character");
	
	bindFunction(0x01F3, game_vehicle_in_air, 1, "Is Vehicle In Air" );
	bindFunction(0x01F4, game_vehicle_flipped, 1, "Is Car Flipped" );
	
	bindFunction(0x0204, game_character_near_car_2d, 5, "Is Char near Car in Car 2D" );
	
	bindUnimplemented( 0x020A, game_lock_vehicle_doors, 2, "Lock Car Doors" );
	
	bindFunction(0x0211, game_navigate_on_foot, 3, "Character go to on foot" );
	
	bindFunction(0x0213, game_create_pickup, 6, "Create pickup");
	bindFunction(0x0214, game_is_pickup_collected, 1, "Has Pickup been collected");
	bindFunction(0x0215, game_destroy_pickup, 1, "Destroy Pickup");
	
	bindFunction(0x0229, game_set_vehicle_colours, 3, "Set Vehicle Colours" );
	
	bindFunction(0x0239, game_character_run_to, 3, "Character Run to" );
	
	bindFunction(0x029B, game_create_object_world, 5, "Create Object no offset" );
	bindFunction(0x029C, game_is_boat, 1, "Is Vehicle Boat" );
	
	bindFunction(0x029F, game_is_character_stopped<PlayerController>, 1, "Is Player Stopped" );
	
	bindFunction(0x02B3, game_character_in_area_9<PlayerController>, 9, "Is Player In Area" );
	
	bindFunction(0x02DE, game_player_in_taxi, 1, "Is Player In Taxi" );
	
	bindFunction(0x02E3, game_get_speed, 2, "Get Vehicle Speed" );

	bindFunction(0x0320, game_character_in_range, 2, "Is Character in range of character");
	
	bindFunction(0x0339, game_objects_in_volume, 11, "Are objects in volume" );
	
	bindFunction( 0x034D, game_rotate_object, 4, "Rotate Object" );
	bindUnimplemented( 0x034E, game_slide_object, 8, "Slide Object" );
	bindFunction(0x034F, game_destroy_object_with_fade<CharacterObject>, 1, "Destroy Character with Fade" );
	
	bindUnimplemented( 0x035D, game_set_object_targetable, 1, "Set Object Targetable" );
	
	bindFunction(0x0363, game_set_close_object_visible, 6, "Set Closest Object Visibility");
	
	bindUnimplemented( 0x0368, game_create_ev_crane, 10, "Create ev Crane" );
	
	bindFunction(0x03B6, game_change_nearest_model, 6, "Change Nearest Instance Model" );
	
	bindUnimplemented( 0x03BA, game_clear_area_vehicles, 6, "Clear Cars From Area" );
	
	bindUnimplemented( 0x03BC, game_create_cylinder, 5, "Create Cylinder" ); /// @todo Other sources call this "sphere", this should be our AreaIndicator?
	bindUnimplemented( 0x03BD, game_destroy_cylinder, 1, "Destroy Cylinder" );

	bindFunction(0x03F3, game_get_vehicle_colours, 3, "Get Vehicle Colours" );
	
	bindUnimplemented(0x03F9, game_make_characters_converse, 3, "Make characters converse");
	
	bindUnimplemented( 0x042B, game_clear_volume_pedestrians, 6, "Clear volume pedestrians" );
	
	bindFunction(0x0442, game_character_in_vehicle<PlayerController>, 2, "Is Player in This Vehicle" );
  bindFunction(0x0443, game_player_in_any_vehicle, 1, "Is Player In Any Vehicle" ); /// Duplicate of 00E0?!
	bindFunction(0x0448, game_character_in_vehicle<CharacterObject>, 2, "Is Character in This Vehicle" );
}
