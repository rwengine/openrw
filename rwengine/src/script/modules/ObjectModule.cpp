#include <script/modules/ObjectModule.hpp>

#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <render/Model.hpp>
#include <render/GameRenderer.hpp>
#include <engine/Animator.hpp>
#include <engine/GameState.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/PickupObject.hpp>
#include <objects/GenericPickup.hpp>
#include <core/Logger.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

glm::vec3 spawnMagic( 0.f, 0.f, 1.f );

void game_create_player(const ScriptArguments& args)
{
	auto id	= args[0].integer;
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	
	if( position.z < -99.f ) {
		position = args.getWorld()->getGroundAtPosition(position);
	}
	
	auto pc = args.getWorld()->createPedestrian(1, position + spawnMagic);
	args.getState()->player = new PlayerController(pc);
	
	*args[4].handle = args.getState()->player;
}

void game_set_character_position(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 position(args[1].real, args[2].real, args[3].real + 1.f);
	controller->getCharacter()->setPosition(position + spawnMagic);
}

bool game_player_in_area_2d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec2 min(args[1].real, args[2].real);
	glm::vec2 max(args[3].real, args[4].real);
	auto player = controller->getCharacter()->getPosition();
	if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
		return true;
	}
	return false;
}

bool game_player_in_area_3d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 min(args[1].real, args[2].real, args[3].real);
	glm::vec3 max(args[4].real, args[5].real, args[6].real);
	auto player = controller->getCharacter()->getPosition();
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
	
	// If there is already a chracter less than this distance away, it will be destroyed.
	const float replaceThreshold = 2.f;
	for( auto it = args.getWorld()->objects.begin();
		it != args.getWorld()->objects.end();
	++it)
		{
			if( it->second->type() == GameObject::Character && glm::distance(position, it->second->getPosition()) < replaceThreshold )
			{
				args.getWorld()->destroyObjectQueued(it->second);
			}
		}
		
		
		auto character = args.getWorld()->createPedestrian(id, position + spawnMagic);
		auto controller = new DefaultAIController(character);
		
		if ( args.getThread()->isMission )
		{
			args.getState()->missionObjects.push_back(character);
		}
		
		*args[5].handle = controller;
}

void game_destroy_character(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if ( controller )
	{
		args.getWorld()->destroyObjectQueued(controller->getCharacter());
	}
}

void game_create_vehicle(const ScriptArguments& args)
{
	auto id	= args[0].integer;
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	position += spawnMagic;
	
	// If there is already a vehicle less than this distance away, it will be destroyed.
	const float replaceThreshold = 1.f;
	for( auto it = args.getWorld()->objects.begin();
		it != args.getWorld()->objects.end();
	++it)
		{
			if( it->second->type() == GameObject::Vehicle && glm::distance(position, it->second->getPosition()) < replaceThreshold )
			{
				args.getWorld()->destroyObjectQueued(it->second);
			}
		}
		
		auto vehicle = args.getWorld()->createVehicle(id, position);
		
		if ( args.getThread()->isMission )
		{
			args.getState()->missionObjects.push_back(vehicle);
		}
		
		*args[4].handle = vehicle;
}

void game_destroy_vehicle(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	args.getWorld()->destroyObjectQueued(vehicle);
}

void game_get_vehicle_position(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		auto vp = vehicle->getPosition();
		*args[1].globalReal = vp.x;
		*args[2].globalReal = vp.y;
		*args[3].globalReal = vp.z;
	}
}

void game_get_character_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	*args[1].handle = controller->getCharacter()->getCurrentVehicle();
}

bool game_character_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	auto vehicle = static_cast<VehicleObject*>(*args[1].handle);
	
	if( controller == nullptr || vehicle == nullptr )
	{
		return false;
	}
	
	return controller->getCharacter()->getCurrentVehicle() == vehicle;
}

bool game_character_in_model(const ScriptArguments& args)
{
	auto vdata = args.getWorld()->data->findObjectType<VehicleData>(args[1].integer);
	if( vdata )
	{
		auto controller = (CharacterController*)(*args[0].handle);
		auto character = controller->getCharacter();
		auto vehicle = character->getCurrentVehicle();
		if ( vehicle ) {
			
			return vehicle->model && vdata->modelName == vehicle->model->name;
		}
	}
	return false;
}

bool game_character_in_any_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle != nullptr;
}

bool game_player_in_area_2d_in_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterController*>(*args[0].handle);
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	
	bool drawCylinder = args[5].integer;
	
	if( character->getCharacter()->getCurrentVehicle() == nullptr )
	{
		return false;
	}
	
	auto vp = character->getCharacter()->getCurrentVehicle()->getPosition();
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
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool drawCylinder = !!args[7].integer;
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	if( ! vehicle ) {
		auto distance = center - controller->getCharacter()->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	if( drawCylinder )
	{
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, center, size);
	}
	
	return false;
}

bool game_character_near_point_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool unkown	= !!args[7].integer;
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	if( vehicle ) {
		auto distance = center - controller->getCharacter()->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_near_character_2D(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	auto target = static_cast<CharacterController*>(*args[1].handle);
	glm::vec2 center(target->getCharacter()->getPosition());
	glm::vec2 size(args[2].real, args[3].real);
	bool unkown = !!args[4].integer;

	auto distance = center - glm::vec2(controller->getCharacter()->getPosition());
	distance /= size;
	return glm::length( distance ) < 1.f;
	
	return false;
}

bool game_character_near_character_in_vehicle_2D(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	auto target = static_cast<CharacterController*>(*args[1].handle);
	glm::vec2 center(target->getCharacter()->getPosition());
	glm::vec2 size(args[2].real, args[3].real);
	bool unkown = !!args[4].integer;
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	if( vehicle ) {
		auto distance = center - glm::vec2(controller->getCharacter()->getPosition());
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_near_point_on_foot_2D(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	glm::vec2 center(args[1].real, args[2].real);
	glm::vec2 size(args[3].real, args[4].real);
	bool unkown = !!args[5].integer;
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	if( !vehicle ) {
		auto distance = center - glm::vec2(controller->getCharacter()->getPosition());
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_dead(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if ( controller )
	{
		return !controller->getCharacter()->isAlive();
	}
	return true;
}

bool game_vehicle_dead(const ScriptArguments& args)
{
	auto controller = static_cast<VehicleObject*>(*args[0].handle);
	return controller == nullptr;
}

bool game_character_in_zone(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	std::string zname(args[1].string);
	
	auto zfind = args.getWorld()->data->zones.find(zname);
	if( zfind != args.getWorld()->data->zones.end() ) {
		auto player = controller->getCharacter()->getPosition();
		auto& min = zfind->second.min;
		auto& max = zfind->second.max;
		if( player.x > min.x && player.y > min.y && player.z > min.z &&
			player.x < max.x && player.y < max.y && player.z < max.z ) {
			return true;
			}
	}
	
	return false;
}

void game_create_character_in_vehicle(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	auto type = args[1].integer;
	auto id = args[2].integer;
	
	auto character = args.getWorld()->createPedestrian(id, vehicle->getPosition() + spawnMagic);
	auto controller = new DefaultAIController(character);
	
	character->setCurrentVehicle(vehicle, 0);
	vehicle->setOccupant(0, character);
	
	*args[3].handle = controller;
}

void game_set_character_heading(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	controller->getCharacter()->setHeading(args[1].real);
}

void game_get_character_heading(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if ( vehicle )
	{
		*args[1].globalReal = 0.f;
	}
}

void game_set_vehicle_heading(const ScriptArguments& args)
{
	auto vehicle = (VehicleObject*)(*args[0].handle);
	vehicle->setHeading(args[1].real);
}

void game_set_object_heading(const ScriptArguments& args)
{
	auto inst = (InstanceObject*)(*args[0].handle);
	inst->setHeading(args[1].real);
}

bool game_vehicle_stopped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		return std::abs( vehicle->physVehicle->getCurrentSpeedKmHour() ) <= 0.01f;
	}
	return false;
}

/// Remove object from cleanup at end of missions.
void game_dont_remove_object(const ScriptArguments& args)
{
	auto object = (GameObject*)(*args[0].handle);
	
	auto& mO = args.getState()->missionObjects;
	mO.erase(std::remove(mO.begin(), mO.end(), object), mO.end());
}

bool game_character_in_area_on_foot(const ScriptArguments& args)
{
	/// @todo
	return false;
}

bool game_character_stoped_in_volume_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	bool drawCylinder = !!args[7].integer;
	
	if( controller && controller->getCharacter()->getCurrentVehicle() != nullptr )
	{
		glm::vec3 vec1(args[1].real, args[2].real, args[3].real);
		glm::vec3 vec2(args[4].real, args[5].real, args[6].real);
		glm::vec3 min = glm::min(vec1, vec2);
		glm::vec3 max = glm::max(vec1, vec2);
		
		glm::vec3 pp = controller->getCharacter()->getCurrentVehicle()->getPosition();
		
		if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
			pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
		{
			return controller->getCharacter()->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
		}
		
		// Request the renderer draw a cylinder here.
		if( drawCylinder )
		{
			args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, (max+min)/2.f, (max-min)/2.f);
		}
	}
	return false;
}


bool game_character_stoped_in_volume(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	glm::vec3 vec1(args[1].real, args[2].real, args[3].real);
	glm::vec3 vec2(args[4].real, args[5].real, args[6].real);
	bool drawCylinder = !!args[7].integer;
	
	glm::vec3 min = glm::min(vec1, vec2);
	glm::vec3 max = glm::max(vec1, vec2);
	
	glm::vec3 pp = controller->getCharacter()->getPosition();
	
	if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
		pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
	{
		if( controller->getCharacter()->getCurrentVehicle() != nullptr )
		{
			return controller->getCharacter()->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
		}
		else
		{
			return controller->getCurrentActivity() == nullptr;
		}
	}
	
	if( drawCylinder )
	{
		args.getWorld()->drawAreaIndicator(AreaIndicatorInfo::Cylinder, (max+min)/2.f, (max-min)/2.f);
	}
	
	return false;
}


bool game_is_character_stopped(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if( controller && controller->getCharacter()->getCurrentVehicle() != nullptr )
	{
		return controller->getCharacter()->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.75f;
	}
	
	return true;
}


bool game_character_in_area_9(const ScriptArguments& args)
{
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
	
	for(auto& pair : args.getWorld()->objects)
	{
		GameObject* object = pair.second;
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
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle && (vehicle->vehicle->classType & VehicleData::TAXI) == VehicleData::TAXI;
}

void game_get_speed(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	if( vehicle )
	{
		*args[1].globalReal = vehicle->physVehicle->getCurrentSpeedKmHour();
	}
}

void game_enter_as_driver(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	// Cancel whatever we're currently trying to do.
	controller->skipActivity();
	controller->setNextActivity(new Activities::EnterVehicle(vehicle,0));
}

void game_enter_as_passenger(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	
	// Cancel whatever we're currently trying to do.
	controller->skipActivity();
	/// @todo find next lowest free seat.
	controller->setNextActivity(new Activities::EnterVehicle(vehicle,Activities::EnterVehicle::ANY_SEAT));
}

void game_character_exit_vehicle(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	auto cvehcile = controller->getCharacter()->getCurrentVehicle();
	
	if( cvehcile && cvehcile == vehicle )
	{
		controller->setNextActivity(new Activities::ExitVehicle());
	}
}

void game_character_follow_character(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto leader = (CharacterController*)(*args[1].handle);
	
	controller->setGoal(CharacterController::FollowLeader);
	controller->setTargetCharacter(leader->getCharacter());
}

void game_navigate_on_foot(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 target(args[1].real, args[2].real, 0.f);
	target = args.getWorld()->getGroundAtPosition(target);
	
	controller->skipActivity();
	
	if( controller->getCharacter()->getCurrentVehicle() )
	{
		// Since we just cleared the Activities, this will become current immediatley.
		controller->setNextActivity(new Activities::ExitVehicle);
	}
	
	controller->setNextActivity(new Activities::GoTo(target));
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
	
	
	auto pickup = new GenericPickup(args.getWorld(), pos, id, type);
	
	args.getWorld()->insertObject( pickup );
	
	*args[5].handle = pickup;
}

bool game_is_pickup_collected(const ScriptArguments& args)
{
	PickupObject* pickup = args[0].handleOf<PickupObject>();
	
	if ( pickup )
	{
		return pickup->isCollected();
	}
	
	return false;
}

void game_destroy_pickup(const ScriptArguments& args)
{
	PickupObject* pickup = args[0].handleOf<PickupObject>();
	
	if ( pickup )
	{
		args.getWorld()->destroyObjectQueued(pickup);
	}
}

void game_character_run_to(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 target(args[1].real, args[2].real, 0.f);
	target = args.getWorld()->getGroundAtPosition(target);
	
	controller->setNextActivity(new Activities::GoTo(target));
}

bool game_vehicle_flipped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		return vehicle->isFlipped();
	}
	
	return false;
}

bool game_vehicle_in_air(const ScriptArguments& args)
{
	/// @todo IS vehicle in air.
	auto vehicle = (VehicleObject*)(*args[0].handle);
	return false;	
}

bool game_character_near_car_2d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	glm::vec2 radius(args[2].real, args[3].real);
	bool drawMarker = !!args[4].integer;
	
	auto charVehicle = controller->getCharacter()->getCurrentVehicle();
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
	auto vehicle = (VehicleObject*)(*args[0].handle);
	
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

	*args[4].handle = inst;
}

void game_destroy_object(const ScriptArguments& args)
{
	auto object = static_cast<GameObject*>(*args[0].handle);
	
	args.getWorld()->destroyObjectQueued(object);
}

bool game_is_boat(const ScriptArguments& args)
{
	/*auto vehicle = (VehicleObject*)(*args[0].handle);
	 *	if( vehicle )
	 *	{
	 *		return vehicle->vehicle->type == VehicleData::BOAT;
}*/
	return false;
}

bool game_character_in_range(const ScriptArguments& args)
{
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
	}
	
	if( std::abs(modelid) > 178 ) {
		/// @todo implement this path,
		return;
	}
	
	std::string model;
	
	if(modelid < 0) modelid = -modelid;
	
	model = args.getVM()->getFile()->getModels()[modelid];
	
	std::transform(model.begin(), model.end(), model.begin(), ::tolower);
	
	for(auto& p : args.getWorld()->objects) {
		auto o = p.second;
		if( o->type() == GameObject::Instance ) {
			if( !o->model ) continue;
			if( o->model->name != model ) continue;
			float d = glm::distance(position, o->getPosition());
			if( d < radius ) {
				o->visible = !!args[5].integer;
			}
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
	}
	
	switch(args[5].type) {
		case TInt8:
			newid = (std::int8_t)args[5].integer;
			break;
		case TInt16:
			newid = (std::int16_t)args[5].integer;
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
	for(auto p : args.getWorld()->objects) {
		auto o = p.second;
		if( o->type() == GameObject::Instance ) {
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
}

bool game_rotate_object(const ScriptArguments& args)
{
	auto object = static_cast<GameObject*>(*args[0].handle);
	float start = args[1].real;
	float finish = args[2].real;
	
	// @todo INTERPOLATE instead of just setting the heading.
	object->setHeading(finish);
	
	return true;
}

void game_get_vehicle_colours(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
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
	
	bindFunction(0x0055, game_set_character_position, 4, "Set Player Position" );
	bindFunction(0x0056, game_player_in_area_2d, 6, "Is Player In Area 2D" );
	bindFunction(0x0057, game_player_in_area_3d, 8, "Is Player In Area 3D" );
	
	bindFunction(0x009A, game_create_character, 6, "Create Character" );
	bindFunction(0x009B, game_destroy_character, 1, "Destroy Character" );
	
	bindUnimplemented( 0x009F, game_character_make_idle, 1, "Set Character to Idle" );
	
	bindFunction(0x00A1, game_set_character_position, 4, "Set Character Position" );
	
	bindFunction(0x00A5, game_create_vehicle, 5, "Create Vehicle" );
	bindFunction(0x00A6, game_destroy_vehicle, 1, "Destroy Vehicle" );
	
	bindFunction(0x00AA, game_get_vehicle_position, 4, "Get Vehicle Position" );
	
	bindFunction(0x00D9, game_get_character_vehicle, 2, "Get Character Vehicle" );
	bindFunction(0x00DA, game_get_character_vehicle, 2, "Get Player Vehicle" );
	bindFunction(0x00DB, game_character_in_vehicle, 2, "Is Character in Vehicle" );
	bindFunction(0x00DC, game_character_in_vehicle, 2, "Is Player in Vehicle" );
	
	bindFunction(0x00DE, game_character_in_model, 2, "Is Player In Model" );
	bindFunction(0x00DF, game_character_in_any_vehicle, 1, "Is Character Driving" );
	
	bindFunction(0x00E0, game_character_in_any_vehicle, 1, "Is Player In Any Vehicle" );
	
	bindFunction(0x00E5, game_player_in_area_2d_in_vehicle, 6, "Is Player in 2D Area in Vehicle" );
	
	bindUnimplemented( 0x00E4, game_locate_character_on_foot_2d, 6, "Locate Player on foot 2D" );
	
	bindFunction(0x00E9, game_character_near_character_2D, 5, "Locate Character near Character 2D");
	
	bindFunction(0x00EB, game_character_near_character_in_vehicle_2D, 5, "Is player near character in vehicle" );
	
	bindFunction(0x00ED, game_character_near_point_on_foot_2D, 6, "Is Character near point on foot" );
	
	bindUnimplemented( 0x00F5, game_locate_character_in_sphere, 8, "Locate Player In Sphere" );
	bindFunction(0x00F6, game_character_near_point_on_foot_3D, 8, "Is Character near point on foot" );
	
	bindFunction(0x0100, game_character_near_point_in_vehicle, 8, "Is Character near point in car" );
	
	bindFunction(0x0108, game_destroy_object, 1, "Destroy Object" );
	
	bindFunction(0x0118, game_character_dead, 1, "Is Character Dead" );
	bindFunction(0x0119, game_vehicle_dead, 1, "Is Vehicle Dead" );
	
	bindUnimplemented(0x011C, game_character_clear_objective, 1, "Clear Character Objective" );
	
	bindFunction(0x0121, game_character_in_zone, 2, "Is Player In Zone" );
	
	bindFunction(0x0129, game_create_character_in_vehicle, 4, "Create Character In Car" );

	bindUnimplemented(0x0135, game_set_vehicle_locked, 2, "Set Vehicle locked state");
	
	bindFunction(0x0171, game_set_character_heading, 2, "Set Player Heading" );
	
	bindFunction(0x0173, game_set_character_heading, 2, "Set Character Heading" );
	bindFunction(0x0174, game_get_character_heading, 2, "Get Vehicle Heading" );
	
	bindFunction(0x0175, game_set_vehicle_heading, 2, "Set Vehicle heading" );
	
	bindFunction(0x0177, game_set_object_heading, 2, "Set Object heading" );
	
	bindUnimplemented( 0x0192, game_character_stand_still, 1, "Make character stand still" );
	
	bindFunction(0x019C, game_character_in_area_on_foot, 8, "Is Player in Area on Foot" );
	bindFunction(0x019E, game_character_stoped_in_volume, 8, "Is Player stopped in volume" );
	
	bindFunction(0x01A0, game_character_stoped_in_volume_in_vehicle, 8, "Is Player Stopped in cube in vehicle" );
	bindFunction(0x01A8, game_character_stoped_in_volume, 8, "Is Char Stopped in volume" );
	bindFunction(0x01AA, game_character_stoped_in_volume_in_vehicle, 8, "Is Char Stopped in cube in vehicle" );
	
	bindUnimplemented( 0x01BB, game_object_coordinates, 4, "Get Object Coordinates" );
	
	bindUnimplemented( 0x01BE, game_turn_character, 4, "Turn Character To Face Point" );
	
	bindFunction(0x01C1, game_vehicle_stopped, 1, "Is Vehicle Stopped" );
	
	bindUnimplemented( 0x01C3, game_release_vehicle, 1, "Mark Car Unneeded" );
	
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
	
	bindFunction(0x029F, game_is_character_stopped, 1, "Is Player Stopped" );
	
	bindFunction(0x02B3, game_character_in_area_9, 9, "Is Player In Area" );
	
	bindFunction(0x02DE, game_player_in_taxi, 1, "Is Player In Taxi" );
	
	bindFunction(0x02E3, game_get_speed, 2, "Get Vehicle Speed" );
	
	bindUnimplemented( 0x02FB, game_create_crane, 10, "Create Crusher Crane" );
	
	bindFunction(0x0320, game_character_in_range, 2, "Is Character in range of character");
	
	bindFunction(0x0339, game_objects_in_volume, 11, "Are objects in volume" );
	
	bindFunction( 0x034D, game_rotate_object, 4, "Rotate Object" );
	bindUnimplemented( 0x034E, game_slide_object, 8, "Slide Object" );
	
	bindUnimplemented( 0x035D, game_set_object_targetable, 1, "Set Object Targetable" );
	
	bindFunction(0x0363, game_set_close_object_visible, 6, "Set Closest Object Visibility");
	
	bindUnimplemented( 0x0368, game_create_ev_crane, 10, "Create ev Crane" );
	
	bindFunction(0x03B6, game_change_nearest_model, 6, "Change Nearest Instance Model" );
	
	bindUnimplemented( 0x03BA, game_clear_area_vehicles, 6, "Clear Cars From Area" );
	
	bindFunction(0x03F3, game_get_vehicle_colours, 3, "Get Vehicle Colours" );
	
	bindUnimplemented(0x03F9, game_make_characters_converse, 3, "Make characters converse");
	
	bindUnimplemented( 0x042B, game_clear_volume_pedestrians, 6, "Clear volume pedestrians" );
	
	bindFunction(0x0442, game_character_in_vehicle, 2, "Is Player in This Vehicle" );
	bindFunction(0x0448, game_character_in_vehicle, 2, "Is Character in This Vehicle" );
}
