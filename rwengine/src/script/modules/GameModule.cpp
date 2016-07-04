#include "script/modules/GameModule.hpp"
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <data/Model.hpp>
#include <engine/Animator.hpp>
#include <engine/GameState.hpp>
#include <engine/GameData.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/PickupObject.hpp>
#include <core/Logger.hpp>
#include <engine/ScreenText.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>


// Helper function to format script numbers to strings
// for use in the text printing opcodes.
std::string formatValue(const SCMOpcodeParameter& p)
{
	switch (p.type) {
	case TFloat16:
		return std::to_string(p.real);
	default:
		return std::to_string(p.integerValue());
	}
	return "";
}

void game_print_big(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getWorld()->data->texts.text(id);
	unsigned short time = args[1].integer;
	unsigned short style = args[2].integer;
	args.getWorld()->state->text.addText<ScreenTextType::Big>(
				ScreenTextEntry::makeBig(
					id, str, style, time
					));
}

void game_print_now(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getWorld()->data->texts.text(id);
	int time = args[1].integer;
	int flags = args[2].integer;
	RW_UNUSED(flags);
	RW_UNIMPLEMENTED("Unclear what style should be used");
	args.getWorld()->state->text.addText<ScreenTextType::HighPriority>(
				ScreenTextEntry::makeHighPriority(
					id, str, time
					));
}

void game_print_soon(const ScriptArguments& args)
{
	const auto& world = args.getWorld();

	std::string id(args[0].string);
	int time = args[1].integerValue();
	unsigned short style = args[2].integerValue();

	auto str = world->data->texts.text(id);

	auto textEntry = ScreenTextEntry::makeBig(id, str, style, time);
	world->state->text.addText<ScreenTextType::BigLowPriority>(textEntry);
}

void game_clear_prints(const ScriptArguments& args)
{
	args.getWorld()->state->text.clear<ScreenTextType::Big>();
}

void game_get_time(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getWorld()->getHour();
	*args[1].globalInteger = args.getWorld()->getMinute();
}

void game_set_time(const ScriptArguments& args)
{
	args.getWorld()->state->basic.gameHour = args[0].integer;
	args.getWorld()->state->basic.gameMinute = args[1].integer;
}

bool game_is_button_pressed(const ScriptArguments& args)
{
	int player = args[0].integerValue();
	int index = args[1].integerValue();
	RW_UNIMPLEMENTED("game_is_button_pressed()");
	// NOTE: This is a hack. Hence we'll keep the unimplemented message for now.
	if (player == 0) {
		if (index == 19) { // look behind / sub-mission
			/// @todo Return the keystate instead
			auto object = args.getWorld()->pedestrianPool.find(args.getState()->playerObject);
			auto player = static_cast<CharacterObject*>(object);
			return player->isRunning();
		}
	}
	return false;
}

void game_set_dead_or_arrested(const ScriptArguments& args)
{
	*args.getWorld()->state->scriptOnMissionFlag = args[0].integer;
}

bool game_has_death_or_arrest_finished(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_has_death_or_arrest_finished()");
	return false;
}

void game_create_vehicle_generator(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	
	if(args[4].type == TString)
	{
		args.getWorld()->logger->error("SCM", "Model names not supported for vehicle generator");
		return;
	}
	
	float heading = args[3].real;
	int vehicleID = args[4].integer;
	int colourFG = args[5].integer;
	int colourBG = args[6].integer;
	bool alwaysSpawn = args[7].integer != 0;
	short alarmThreshold = args[8].integer;
	short lockedThreshold = args[9].integer;
	int minDelay = args[10].integer;
	int maxDelay = args[11].integer;

	VehicleGenerator vg{0,position,
	                    heading,
	                    vehicleID,
	                    colourFG,
	                    colourBG,
	                    alwaysSpawn,
	                    alarmThreshold,
	                    lockedThreshold,
	                    minDelay,
	                    maxDelay,
	                    0,
	                    0};

	*args[12].globalInteger = args.getWorld()->state->vehicleGenerators.size();
	
	args.getWorld()->state->vehicleGenerators.push_back(vg);
}
void game_set_vehicle_generator_count(const ScriptArguments& args)
{
	RW_CHECK(args.getWorld()->state->vehicleGenerators.size() > *args[0].globalInteger, "VehicleGenerator out of range");
	if ((int)args.getWorld()->state->vehicleGenerators.size() > *args[0].globalInteger) {
		VehicleGenerator& generator = args.getWorld()->state->vehicleGenerators.at(*args[0].globalInteger);
		generator.remainingSpawns = args[1].integer;
	}
}

void game_set_zone_car_info(const ScriptArguments& args)
{
	auto it = args.getWorld()->data->zones.find(args[0].string);
	if( it != args.getWorld()->data->zones.end() )
	{
		auto day = args[1].integer == 1;
		for (size_t i = 2; i < args.getParameters().size() && i - 2 < ZONE_GANG_COUNT; ++i)
		{
			if( day )
			{
				it->second.gangCarDensityDay[i-2] = args[i].integer;
			}
			else
			{
				it->second.gangCarDensityNight[i-2] = args[i].integer;
			}
		}
	}
}

void game_camera_follow_character(const ScriptArguments& args)
{
	auto character = static_cast<CharacterObject*>(args.getObject<CharacterObject>(0));
	if( character != nullptr )
	{
		args.getWorld()->state->cameraTarget = character->getGameObjectID();
	}
}

void game_reset_camera(const ScriptArguments& args)
{
	args.getWorld()->state->cameraTarget = 0;
	args.getWorld()->state->cameraFixed = false;
}

void game_set_zone_ped_info(const ScriptArguments& args)
{
	auto it = args.getWorld()->data->zones.find(args[0].string);
	if( it != args.getWorld()->data->zones.end() )
	{
		auto day = args[1].integer == 1;
		for(size_t i = 2; i < args.getParameters().size(); ++i)
		{
			if( day )
			{
				it->second.gangDensityDay[i-2] = args[i].integer;
			}
			else
			{
				it->second.gangDensityNight[i-2] = args[i].integer;
			}
		}
	}
}

void game_camera_fixed_position(const ScriptArguments& args)
{
	glm::vec3 position( args[0].real, args[1].real, args[2].real );
	glm::vec3 angles( args[3].real, args[4].real, args[5].real );
	
	args.getWorld()->state->cameraFixed = true;
	args.getWorld()->state->cameraPosition = position;
	args.getWorld()->state->cameraRotation = glm::quat(angles);
}

void game_camera_lookat_position(const ScriptArguments& args)
{
	glm::vec3 position( args[0].real, args[1].real, args[2].real );
	int switchmode = args[3].integer;
	RW_UNUSED(switchmode);
	RW_UNIMPLEMENTED("game_camera_lookat_position(): camera switch mode");

	
	auto direction = glm::normalize(position - args.getWorld()->state->cameraPosition);
	auto right = glm::normalize(glm::cross(glm::vec3(0.f, 0.f, 1.f), direction));
	auto up = glm::normalize(glm::cross(direction, right));
	
	glm::mat3 v;
	v[0][0] = direction.x;
	v[0][1] = right.x;
	v[0][2] = up.x;
	
	v[1][0] = direction.y;
	v[1][1] = right.y;
	v[1][2] = up.y;
	
	v[2][0] = direction.z;
	v[2][1] = right.z;
	v[2][2] = up.z;
	
	args.getWorld()->state->cameraRotation = glm::inverse(glm::quat_cast(v));
}

void game_remove_blip(const ScriptArguments& args)
{
	int blip = *args[0].globalInteger;
	args.getWorld()->state->removeBlip(blip);
}

void game_set_fade_colour(const ScriptArguments& args)
{
	args.getWorld()->state->fadeColour.r = args[0].integer;
	args.getWorld()->state->fadeColour.g = args[1].integer;
	args.getWorld()->state->fadeColour.b = args[2].integer;
}
void game_fade_screen(const ScriptArguments& args)
{
	args.getWorld()->state->fadeTime = args[0].integer / 1000.f;
	args.getWorld()->state->fadeOut = !!args[1].integer;
	args.getWorld()->state->fadeStart = args.getWorld()->getGameTime();
}
bool game_screen_fading(const ScriptArguments& args)
{
	if( args.getWorld()->state->skipCutscene )
	{
		return false;
	}
	return args.getWorld()->getGameTime() <
		args.getWorld()->state->fadeStart + args.getWorld()->state->fadeTime;
}

void game_override_restart(const ScriptArguments& args)
{
	args.getWorld()->state->overrideNextStart = true;
	args.getWorld()->state->nextRestartLocation = glm::vec4(
		args[0].real, args[1].real, args[2].real, args[3].real
	);
}

void game_clear_override(const ScriptArguments& args)
{
	args.getWorld()->state->overrideNextStart = false;
}

void game_link_mission_flag(const ScriptArguments& args)
{
	args.getWorld()->state->scriptOnMissionFlag = args[0].globalInteger;
}

template <class Tobject>
void game_add_object_blip(const ScriptArguments& args)
{
	BlipData data;
	auto target = args.getObject<Tobject>(0);
	data.target = target->getGameObjectID();

	switch(target->type()) {
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

	data.texture = "";
	*args[1].globalInteger = args.getWorld()->state->addRadarBlip(data);
}

void game_add_location_blip(const ScriptArguments& args)
{
	BlipData data;
	data.target = 0;
	/// @todo this might use ground coords if z is -100.0
	data.coord = glm::vec3(args[0].realValue(), args[1].realValue(), args[2].realValue());
	data.texture = "";
	*args[3].globalInteger = args.getWorld()->state->addRadarBlip(data);
}

void game_change_blip_mode(const ScriptArguments& args)
{
	int id = *args[0].globalInteger;
	BlipData& blip = args.getWorld()->state->radarBlips[id];
	int mode = args[1].integer;
	
	switch ( mode )
	{
		default:
			blip.display = BlipData::Hide;
			break;
		case 2:
			blip.display = BlipData::RadarOnly;
			break;
		case 3:
			blip.display = BlipData::Show;
			break;
	}
}

void game_enable_input(const ScriptArguments& args)
{
    auto player = static_cast<CharacterObject*>(args.getPlayerCharacter(0));
	static_cast<PlayerController*>(player->controller)->setInputEnabled(!!args[1].integer);
}

void game_set_weather(const ScriptArguments& args)
{
	args.getWorld()->state->basic.nextWeather = args[0].integer;
}

void game_get_runtime(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getWorld()->getGameTime() * 1000;
}

void game_print_big_with_number(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str =
			ScreenText::format(
				args.getWorld()->data->texts.text(id),
				formatValue(args[1]));
	unsigned short style = args[3].integer;
	args.getWorld()->state->text.addText<ScreenTextType::Big>(
				ScreenTextEntry::makeBig(
					id, str, style, 5000
					));
}

void game_disable_roads(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getWorld()->enableAIPaths(AIGraphNode::Vehicle, min, max);
}
void game_enabled_roads(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getWorld()->disableAIPaths(AIGraphNode::Vehicle, min, max);
}

void game_max_wanted_level(const ScriptArguments& args)
{
	args.getWorld()->state->maxWantedLevel = args[0].integer;
}

void game_get_player(const ScriptArguments& args)
{
    auto character = args.getPlayerCharacter(0);
	*args[1].globalInteger = character->getGameObjectID();
}

void game_create_garage(const ScriptArguments& args)
{
	// http://www.gtamodding.com/index.php?title=Garage#GTA_III
	glm::vec3 min(args[0].real, args[1].real, args[2].real);
	glm::vec3 max(args[3].real, args[4].real, args[5].real);
	int garageType = args[6].integer;
	
	args.getWorld()->state->garages.push_back({
		0, min, max, garageType
	});
	int garageIndex = args.getWorld()->state->garages.size() - 1;
	
	*args[7].globalInteger = garageIndex;
}

bool game_is_car_inside_garage(const ScriptArguments& args)
{
	/// @todo move to garage code

	GameWorld* gw = args.getWorld();
	const auto& garages = gw->state->garages;
	int garageIndex = args[0].integerValue();

	RW_CHECK(garageIndex >= 0, "Garage index too low");
	RW_CHECK(garageIndex < static_cast<int>(garages.size()), "Garage index too high");
	const auto& garage = garages[garageIndex];

	for(auto& v : gw->vehiclePool.objects) {
		// @todo if this car only accepts mission cars we probably have to filter here / only check for one specific car
		auto vp = v.second->getPosition();
		if (vp.x >= garage.min.x && vp.y >= garage.min.y && vp.z >= garage.min.z &&
		    vp.x <= garage.max.x && vp.y <= garage.max.y && vp.z <= garage.max.z) {
			return true;
		}
	}

	return false;
}

bool game_garage_contains_car(const ScriptArguments& args)
{
	/// @todo move to garage code

	GameWorld* gw = args.getWorld();
	const auto& garages = gw->state->garages;
	int garageIndex = args[0].integerValue();

	RW_CHECK(garageIndex >= 0, "Garage index too low");
	RW_CHECK(garageIndex < static_cast<int>(garages.size()), "Garage index too high");
	const auto& garage = garages[garageIndex];

	auto vehicle = static_cast<VehicleObject*>(args.getObject<VehicleObject>(1));
	if (vehicle) {
		/// @todo if this car only accepts mission cars we probably have to filter here / only check for one specific car
		auto vp = vehicle->getPosition();
		if(vp.x >= garage.min.x && vp.y >= garage.min.y && vp.z >= garage.min.z &&
		   vp.x <= garage.max.x && vp.y <= garage.max.y && vp.z <= garage.max.z) {
			return true;
		}
	}

	return false;
}

void game_disable_ped_paths(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getWorld()->enableAIPaths(AIGraphNode::Pedestrian, min, max);
}
void game_enable_ped_paths(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getWorld()->disableAIPaths(AIGraphNode::Pedestrian, min, max);
}

void game_load_special_char(const ScriptArguments& args)
{
	args.getWorld()->loadSpecialCharacter(args[0].integer, args[1].string);
}
bool game_special_char_loaded(const ScriptArguments& args)
{
	auto chartype = args.getWorld()->data->findObjectType<CharacterData>(args[0].integer);
	if( chartype ) {
		auto modelfind = args.getWorld()->data->models.find(chartype->modelName);
		if( modelfind != args.getWorld()->data->models.end() && modelfind->second->resource != nullptr ) {
			return true;
		}
	}

	return true;
}

void game_cutscene_offset(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	if( args.getWorld()->state->currentCutscene ) {
		args.getWorld()->state->currentCutscene->meta.sceneOffset = position;
	}
}

bool game_model_loaded(const ScriptArguments& args)
{
	/// @todo this will need changing when model loading is overhauled.
	if( args[0].integer == 0 ) {
		/// @todo Figure out if this really does mean the player.
		return true;
	}
	//auto model = args.getVM()->getFile()->getModels()[args[0].integer];
	//if( model == "" ) return true; // ??
	return true;
}

void game_restart_critical_mission(const ScriptArguments& args)
{
	args.getWorld()->logger->info("SCM", "Restarting Critical Mission");
	// Reset player state.
	glm::vec3 position(args[0].real, args[1].real, args[2].real + 1.f);
	
	auto object = args.getWorld()->pedestrianPool.find(args.getState()->playerObject);
	auto player = static_cast<CharacterObject*>(object);
	
	glm::vec3 spawnMagic( 0.f, 0.f, 1.f );
	
	player->setPosition(position + spawnMagic);
	
	player->setHeading( args[3].real );
	
	/// @todo find a nicer way to implement warping out of vehicles.
	auto cv = player->getCurrentVehicle();
	if ( cv != nullptr )
	{
		cv->setOccupant( player->getCurrentSeat(), nullptr );
		player->setCurrentVehicle(nullptr, 0);
	}
}

/// @todo http://www.gtamodding.com/index.php?title=0256 (e.g. check if dead or busted)
bool game_is_player_playing(const ScriptArguments& args)
{
    auto character = args.getPlayerCharacter(0);
	return character != nullptr;
	//return args.getWorld()->findObject(args.getState()->playerObject) != nullptr;
}

void game_controller_mode(const ScriptArguments& args)
{
	*args[0].globalInteger	= 0;
}

void game_set_widescreen(const ScriptArguments& args)
{
	args.getWorld()->state->isCinematic = !!args[0].integer;
}

static const char* sprite_names[] = {
	"", // 0
	"radar_asuka",
	"radar_bomb",
	"radar_cat",
	"radar_centre",
	"radar_copcar",
	"radar_don",
	"radar_eight",
	"radar_el",
	"radar_ice",
	"radar_joey",
	"radar_kenji",
	"radar_liz",
	"radar_luigi",
	"radar_north",
	"radar_ray",
	"radar_sal",
	"radar_save",
	"radar_spray",
	"radar_tony",
	"radar_weapon",
};

void game_add_contact_blip(const ScriptArguments& args)
{
	glm::vec3 c( args[0].real, args[1].real, args[2].real );
	unsigned int sprite = args[3].integer;
	
	// Look up the sprite ID.
	std::string spriteName = "";
	if ( sprite < sizeof( sprite_names ) )
	{
		spriteName = sprite_names[sprite];
	}
	
	BlipData bd;
	bd.coord = c;
	bd.target = 0;
	bd.texture = spriteName;
	
	*args[4].globalInteger = args.getWorld()->state->addRadarBlip(bd);
}

void game_add_sprite_blip(const ScriptArguments& args)
{
	glm::vec3 c( args[0].real, args[1].real, args[2].real );
	unsigned int sprite = args[3].integer;
	
	// Look up the sprite ID.
	std::string spriteName = "";
	if ( sprite < sizeof( sprite_names ) )
	{
		spriteName = sprite_names[sprite];
	}
	
	BlipData bd;
	bd.coord = c;
	bd.target = 0;
	bd.texture = spriteName;
	
	*args[4].globalInteger = args.getWorld()->state->addRadarBlip(bd);
}

template <class Tobject>
void game_add_object_sprite_blip(const ScriptArguments& args)
{
	BlipData data;
	auto target = args.getObject<Tobject>(0);
	data.target = target->getGameObjectID();
	unsigned int sprite = args[1].integerValue();

	// Look up the sprite ID.
	std::string spriteName = "";
	if ( sprite < sizeof( sprite_names ) )
	{
		spriteName = sprite_names[sprite];
	}

	switch(target->type()) {
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

	data.texture = spriteName;
	*args[2].globalInteger = args.getWorld()->state->addRadarBlip(data);
}

void game_load_cutscene(const ScriptArguments& args)
{
	args.getWorld()->loadCutscene(args[0].string);
	args.getWorld()->state->cutsceneStartTime = -1.f;
}
void game_create_cutscene_object(const ScriptArguments& args)
{
	auto id	= args[0].integer;

	GameObject* object = args.getWorld()->createCutsceneObject(id, args.getWorld()->state->currentCutscene->meta.sceneOffset );

	if( object == nullptr ) {
		args.getWorld()->logger->error("SCM", "Could not create cutscene object " + std::to_string(id));
	}
	else
	{
		*args[1].globalInteger = object->getGameObjectID();
	}
}
void game_set_cutscene_anim(const ScriptArguments& args)
{
	GameObject* object = args.getObject<CutsceneObject>(0);
	std::string animName = args[1].string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = args.getWorld()->data->animations[animName];
	if( anim ) {
		object->animator->playAnimation(0, anim, 1.f, false);
	}
	else {
		args.getWorld()->logger->error("SCM", "Failed to load cutscene anim: " + animName);
	}
}
void game_start_cutscene(const ScriptArguments& args)
{
	args.getWorld()->startCutscene();
}
void game_get_cutscene_time(const ScriptArguments& args)
{
	float time = args.getWorld()->getGameTime() - args.getWorld()->state->cutsceneStartTime;
	if( args.getWorld()->state->skipCutscene )
	{
		*args[0].globalInteger = args.getWorld()->state->currentCutscene->tracks.duration * 1000;
	}
	else
	{
		*args[0].globalInteger = time * 1000;
	}
}
bool game_cutscene_finished(const ScriptArguments& args)
{
	if( args.getWorld()->state->currentCutscene ) {
		float time = args.getWorld()->getGameTime() - args.getWorld()->state->cutsceneStartTime;
		if( args.getWorld()->state->skipCutscene ) {
			return true;
		}
		return time > args.getWorld()->state->currentCutscene->tracks.duration;
	}
	return true;
}
void game_clear_cutscene(const ScriptArguments& args)
{
	args.getWorld()->clearCutscene();
}

void game_set_hidden_packages(const ScriptArguments& args)
{
	args.getWorld()->state->playerInfo.hiddenPackageCount = args[0].integer;
}

void game_load_special_model(const ScriptArguments& args)
{
	args.getWorld()->loadSpecialModel(args[0].integer, args[1].string);
}
void game_create_cutscene_head(const ScriptArguments& args)
{
	auto id = args[1].integer;
	auto actor = args.getObject<CutsceneObject>(0);
	CutsceneObject* object = args.getWorld()->createCutsceneObject(id, args.getWorld()->state->currentCutscene->meta.sceneOffset );

	auto headframe = actor->model->resource->findFrame("shead");
	actor->skeleton->setEnabled(headframe, false);
	object->setParentActor(actor, headframe);

	*args[2].globalInteger = object->getGameObjectID();
}
void game_set_head_animation(const ScriptArguments& args)
{
	GameObject* object = args.getObject<CutsceneObject>(0);
	std::string animName = args[1].string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = args.getWorld()->data->animations[animName];
	if( anim ) {
		object->animator->playAnimation(0, anim, 1.f, false);
	}
	else {
		args.getWorld()->logger->error("SCM", "Failed to load cutscene anim: " + animName);
	}
}

void game_create_crusher_crane(const ScriptArguments& args)
{
	glm::vec2 crane_location(args[0].real, args[1].real);
	glm::vec2 park_min(args[2].real, args[3].real);
	glm::vec2 park_max(args[4].real, args[5].real);
	glm::vec3 crusher_position(args[6].real, args[7].real, args[8].real);
	float crusher_heading = args[9].real;

	RW_UNIMPLEMENTED("create_crusher_crane is incomplete");
	/// @todo check how to store all parameters and how to create the actual crusher
	RW_UNUSED(crane_location);
	RW_UNUSED(crusher_position);
	/// @todo check how the savegame stores the heading value etc.
	RW_UNUSED(crusher_heading);

	// NOTE: These values come from a savegame from the original game
	glm::vec3 min(park_min, -1.f);
	glm::vec3 max(park_max, 3.5f);
	int garageType = GarageInfo::GARAGE_CRUSHER;

	// NOTE: This instruction also creates or controls a garage
	/// @todo find out if this creates a garage or if it just controls garage[0]
	args.getWorld()->state->garages.push_back({
		0, min, max, garageType
	});
}

void game_increment_progress(const ScriptArguments& args)
{
	args.getWorld()->state->currentProgress += args[0].integer;
}

void game_set_max_progress(const ScriptArguments& args)
{
	args.getWorld()->state->maxProgress = args[0].integer;
}

void game_set_unique_jumps(const ScriptArguments& args)
{
	args.getWorld()->state->gameStats.uniqueStuntsTotal = args[0].integer;
}

void game_set_last_mission(const ScriptArguments& args)
{
	args.getWorld()->state->lastMissionName = args[0].string;
}

void game_set_zone_ped_group(const ScriptArguments& args)
{
	auto it = args.getWorld()->data->zones.find(args[0].string);
	if( it != args.getWorld()->data->zones.end() )
	{
		auto day = args[1].integer == 1;
		if( day )
		{
			it->second.pedGroupDay = args[2].integer;
		}
		else
		{
			it->second.pedGroupNight = args[2].integer;
		}
	}
}

bool game_has_respray_happened(const ScriptArguments& args)
{
	const auto& garages = args.getWorld()->state->garages;
	int garageIndex = args[0].integerValue();

	RW_CHECK(garageIndex >= 0, "Garage index too low");
	RW_CHECK(garageIndex < static_cast<int>(garages.size()), "Garage index too high");
	const auto& garage = garages[garageIndex];

	if (garage.type != GarageInfo::GARAGE_RESPRAY) {
		return false;
	}

	auto playerobj = args.getWorld()->pedestrianPool.find(args.getState()->playerObject);
	auto pp = playerobj->getPosition();
	if (pp.x >= garage.min.x && pp.y >= garage.min.y && pp.z >= garage.min.z &&
	    pp.x <= garage.max.x && pp.y <= garage.max.y && pp.z <= garage.max.z) {
		return true;
	}

	return false;
}

void game_display_text(const ScriptArguments& args)
{
	glm::vec2 pos(args[0].real, args[1].real);
	std::string str(args[2].string);
	str = args.getWorld()->data->texts.text(str);
	args.getWorld()->state->nextText.text = str;
	args.getWorld()->state->nextText.position = pos;
	args.getWorld()->state->texts.push_back(args.getWorld()->state->nextText);
}

void game_set_text_colour(const ScriptArguments& args)
{
	args.getWorld()->state->nextText.colourFG.r = args[0].integer / 255.f;
	args.getWorld()->state->nextText.colourFG.g = args[1].integer / 255.f;
	args.getWorld()->state->nextText.colourFG.b = args[2].integer / 255.f;
	args.getWorld()->state->nextText.colourFG.a = args[3].integer / 255.f;
}

void game_set_background_colour(const ScriptArguments& args)
{
	args.getWorld()->state->nextText.colourBG.r = args[0].integer / 255.f;
	args.getWorld()->state->nextText.colourBG.g = args[1].integer / 255.f;
	args.getWorld()->state->nextText.colourBG.b = args[2].integer / 255.f;
	args.getWorld()->state->nextText.colourBG.a = args[3].integer / 255.f;
}

void game_set_character_model(const ScriptArguments& args)
{
	auto character = args.getObject<CharacterObject>(0);
	if( character )
	{
		static_cast<CharacterObject*>(character)->changeCharacterModel(args[1].string);
	}
}

void game_start_chase_scene(const ScriptArguments& args)
{
	// Hardcoded list of vehicles, put this somewhere else.

#define CHASE_VEHICLE(model, x, y, z, hdg, c1, c2, path) \
	{ \
		auto vehicle0 = args.getWorld()->createVehicle( \
					model, \
					glm::vec3(x, y, z), \
					glm::angleAxis(glm::radians(hdg), glm::vec3(0.f, 0.f, 1.f))); \
		vehicle0->setPrimaryColour(c1);\
		vehicle0->setSecondaryColour(c2);\
		args.getWorld()->chase.addChaseVehicle(vehicle0, path,\
											   args.getWorld()->data->getDataPath()+"/data/paths/CHASE" #path ".DAT");\
	}

	CHASE_VEHICLE(116,   273.5422f,  -1167.1907f,   24.9906f, 64.f,    2, 1,  0);
	CHASE_VEHICLE(117,   231.1783f,  -1388.832f,    25.9782f, 90.0f,   2, 1,  1);
	CHASE_VEHICLE(130,  -28.9762f,   -1031.3367f,   25.9781f, 242.0f,  1, 75, 2);
	CHASE_VEHICLE(96,    114.1564f,  -796.6938f,    24.9782f, 180.0f,  0, 0,  3);
	CHASE_VEHICLE(110,   184.3156f,  -1473.251f,    25.9782f, 0.0f,    6, 6,  4);
	CHASE_VEHICLE(105,   173.8868f,  -1377.6514f,   25.9782f, 0.0f,    4, 5,  6);
	CHASE_VEHICLE(92,    102.5946f,  -943.9363f,    25.9781f, 270.0f,  53,53, 7);
	CHASE_VEHICLE(105,   177.7157f,  -862.1865f,    25.9782f, 155.0f,  41, 1, 10);
	CHASE_VEHICLE(92,    170.5698f,  -889.0236f,    25.9782f, 154.0f,  10, 10,11);
	CHASE_VEHICLE(111,   402.6081f,  -917.4963f,    37.381f,  90.0f,   34, 1, 14);
	CHASE_VEHICLE(110,  -33.4962f,   -938.4563f,    25.9781f, 266.0f,  6,  6, 16);
	CHASE_VEHICLE(111,   49.3631f,   -987.605f,     25.9781f, 0.0f,    51, 1, 18);
	CHASE_VEHICLE(110,   179.0049f,  -1154.6686f,   25.9781f, 0.0f,    6, 76, 19);

	args.getWorld()->chase.start();
}

void game_print_big_with_2_numbers(const ScriptArguments& args)
{
	const auto& world = args.getWorld();

	auto id(args[0].string);
	int time = args[3].integerValue();
	unsigned short style = args[4].integerValue();

	std::string str = ScreenText::format(world->data->texts.text(id),
	                                     formatValue(args[1]),
	                                     formatValue(args[2]));

	auto textEntry = ScreenTextEntry::makeBig(id, str, style, time);
	world->state->text.addText<ScreenTextType::Big>(textEntry);
}

void game_stop_chase_scene(const ScriptArguments& args)
{
	// Clean up remaining vehicles
	args.getWorld()->chase.cleanup();
}

bool game_collision_loaded(const ScriptArguments& args)
{
	// The paramter to this is actually the island number.
	// Not sure how that will fit into the scheme of full paging
	/// @todo use the current player zone island number to return the correct value.
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_collision_loaded()");
	return true;
}

void game_load_audio(const ScriptArguments& args)
{
	std::string name = args[0].string;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	if ( ! args.getWorld()->data->loadAudioClip(name, name + ".wav")) {
		if ( ! args.getWorld()->data->loadAudioClip(name, name + ".mp3")) {
			args.getWorld()->logger->error("SCM", "Failed to load audio: " + name);
		}
	}
}

bool game_is_audio_loaded(const ScriptArguments& args)
{
	auto world = args.getWorld();
	return world->sound.isLoaded(world->missionAudio);
}

void game_play_mission_audio(const ScriptArguments& args)
{
	auto world = args.getWorld();
	if (world->missionAudio.length() > 0) {
		world->sound.playSound(world->missionAudio);
	}
}
bool game_is_audio_finished(const ScriptArguments& args)
{
	auto world = args.getWorld();
	bool isFinished = ! world->sound.isPlaying(world->missionAudio);

	if (isFinished) {
		world->missionAudio = "";
	}

	return isFinished;
}

bool game_import_garage_contains_needed_car(const ScriptArguments& args)
{
	const auto& garages = args.getWorld()->state->garages;

	int garageIndex = args[0].integerValue();
	RW_CHECK(garageIndex >= 0, "Garage index too low");
	RW_CHECK(garageIndex < static_cast<int>(garages.size()), "Garage index too high");
	const auto& garage = garages[garageIndex];

	int entryIndex = args[1].integerValue();
	RW_CHECK(entryIndex >= 0, "Entry index too low");
	RW_CHECK(entryIndex < 32, "Entry index too high");

	if (garage.type == GarageInfo::GARAGE_COLLECTCARS1) {
		return args.getState()->importExportPortland[entryIndex];
	}
	if (garage.type == GarageInfo::GARAGE_COLLECTCARS2) {
		return args.getState()->importExportShoreside[entryIndex];
	}
	if (garage.type == GarageInfo::GARAGE_COLLECTCARS3) {
		return args.getState()->importExportUnused[entryIndex];
	}

	return false;
}

void game_play_music_id(const ScriptArguments& args)
{
	int id = args[0].integer;
	RW_UNUSED(id);
	RW_UNIMPLEMENTED("game_play_music_id(): should be play mission passed tune");
	GameWorld* gw = args.getWorld();
	std::string name = "Miscom";
	
	// TODO play anything other than Miscom.wav
	if(! gw->data->loadAudioClip( name, name + ".wav" ) )
	{
		args.getWorld()->logger->error("SCM", "Error loading audio " + name);
		return;
	}
	else if (args.getWorld()->missionAudio.length() > 0)
	{
		args.getWorld()->sound.playSound(args.getWorld()->missionAudio);
	}
}

void game_clear_area(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	float radius = args[3].real;
	bool clearParticles = args[4].integer;

	GameWorld* gw = args.getWorld();

	for(auto& v : gw->vehiclePool.objects)
	{
		if( glm::distance(position, v.second->getPosition()) < radius )
		{
			gw->destroyObjectQueued(v.second);
		}
	}

	for(auto& p : gw->pedestrianPool.objects)
	{
		// Hack: Not sure what other objects are exempt from this opcode
		if (p.second->getLifetime() == GameObject::PlayerLifetime) {
			continue;
		}
		if( glm::distance(position, p.second->getPosition()) < radius )
		{
			gw->destroyObjectQueued(p.second);
		}
	}

	/// @todo Do we also have to clear all projectiles + particles *in this area*, even if the bool is false?

	if (clearParticles)
	{
		RW_UNUSED(clearParticles);
		RW_UNIMPLEMENTED("game_clear_area(): should clear all particles and projectiles (not limited to area!)");
	}
}

void game_clear_print(const ScriptArguments& args)
{
	std::string id(args[0].string);

	args.getWorld()->state->text.remove<ScreenTextType::Big>(id);
}

bool game_did_game_save(const ScriptArguments& args)
{
	// TODO not sure what could be false about this.
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_did_game_save(): Saving the game");
	return true;
}

void game_get_found_hidden_packages(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getWorld()->state->playerInfo.hiddenPackagesCollected;
}

void game_display_help(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getWorld()->data->texts.text(id);

	args.getWorld()->state->text.addText<ScreenTextType::Big>(
				ScreenTextEntry::makeHelp(
					id, str
					));
}

void game_clear_help(const ScriptArguments& args)
{
	args.getWorld()->state->text.clear<ScreenTextType::Help>();
}

bool game_has_crane_collected_all_cars(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_has_crane_collected_all_cars()");
	return false;
}

bool game_can_player_move(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_can_player_move()");
	return true;
}

void game_load_collision(const ScriptArguments& args)
{
	// Collision is loaded when required, not sure what this is supposed to mean.
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_load_collision();");
}

void game_set_rampages(const ScriptArguments& args)
{
	args.getWorld()->state->gameStats.totalRampages = args[0].integer;
}

void game_remove_chase_car(const ScriptArguments& args)
{
	int chaseCar = args[0].integer;
	GameObject* car = args.getWorld()->chase.getChaseVehicle(chaseCar);
	RW_CHECK(car != nullptr, "Tried to remove null car from chase");
	if (car == nullptr) return;
	args.getWorld()->chase.removeChaseVehicle(chaseCar);
	args.getWorld()->destroyObject(car);
}

void game_set_near_clip(const ScriptArguments& args)
{
	args.getWorld()->state->cameraNear = args[0].real;
}

void game_set_missions(const ScriptArguments& args)
{
	args.getWorld()->state->gameStats.totalMissions = args[0].integer;
}

void game_set_sound_fade(const ScriptArguments& args)
{
	args.getWorld()->state->fadeSound = !!args[0].integer;
}
void game_set_is_intro_playing(const ScriptArguments& args)
{
	args.getWorld()->state->isIntroPlaying = !!args[0].integer;
}

bool game_are_vehicle_cheats_enabled(const ScriptArguments& args)
{
	RW_UNUSED(args);
	RW_UNIMPLEMENTED("game_are_vehicle_cheats_enabled()");
	return false;
}

void game_load_splash(const ScriptArguments& args)
{
	args.getWorld()->data->loadSplash(args[0].string);
}

GameModule::GameModule()
: ScriptModule("Game")
{
	bindUnimplemented(0x00A7, game_drive_to, 4, "Drive To" );
	
	bindUnimplemented( 0x00AD, game_set_driving_speed, 2, "Set Driving Speed" );
	bindUnimplemented( 0x00AE, game_set_driving_style, 2, "Set Driving Style" );
	
	bindFunction(0x00BA, game_print_big, 3, "Print big" );
	bindFunction(0x00BC, game_print_now, 3, "Print Message Now" );
	bindFunction(0x00BD, game_print_soon, 3, "Print Message Soon" );
	
	bindFunction(0x00BE, game_clear_prints, 0, "Clear Message Prints" );
	bindFunction(0x00BF, game_get_time, 2, "Get Time of Day" );
	bindFunction(0x00C0, game_set_time, 2, "Set Time of Day" );

	bindFunction(0x00E1, game_is_button_pressed, 2, "Is Button Pressed" );

	bindUnimplemented( 0x010D, game_set_wanted_level, 2, "Set Wanted Level" );
	
	bindUnimplemented( 0x0109, game_add_character_money, 2, "Add Character Money" );
	
	bindUnimplemented( 0x0110, game_clear_wanted_level, 1, "Clear Wanted Level" );
	bindFunction(0x0111, game_set_dead_or_arrested, 1, "Set Dead or Arrested" );
	bindFunction(0x0112, game_has_death_or_arrest_finished, 0, "Is Death or Arrest Finished" );

	bindFunction(0x014B, game_create_vehicle_generator, 13, "Create Car Generator" );
	bindFunction(0x014C, game_set_vehicle_generator_count, 2, "Set Car Generator count" );
	bindUnimplemented( 0x014D, add_pager_message, 4, "show pager message" );
	bindUnimplemented( 0x014E, game_show_onscreen_timer, 1, "Display Onscreen Timer" );
	
	bindUnimplemented( 0x014F, game_stop_timer, 1, "Stop Timer" );
	
	bindUnimplemented( 0x0151, game_clear_counter, 1, "Clear Counter" );
	bindFunction(0x0152, game_set_zone_car_info, 17, "Set zone car info" );
	
	bindUnimplemented( 0x0158, game_camera_follow_vehicle, 3, "Camera Follow Vehicle" );
	bindFunction(0x0159, game_camera_follow_character, 3, "Camera Follow Character" );
	
	bindFunction(0x015A, game_reset_camera, 0, "Reset Camera" );
	
	bindFunction(0x015C, game_set_zone_ped_info, 11, "Set zone ped info" );
    bindUnimplemented( 0x015D, game_set_timescale;, 1, "Set Game Timescale" );

	bindFunction(0x015F, game_camera_fixed_position, 6, "Set Fixed Camera Position" );
	bindFunction(0x0160, game_camera_lookat_position, 4, "Point Camera at Point" );
	
	bindFunction(0x0164, game_remove_blip, 1, "Remove Blip");

	bindFunction(0x0169, game_set_fade_colour, 3, "Set Fade Colour" );
	bindFunction(0x016A, game_fade_screen, 2, "Fade Screen" );
	bindFunction(0x016B, game_screen_fading, 0, "Is Screen Fading" );
	bindUnimplemented( 0x016C, game_add_hopsital, 4, "Add Hospital Restart" );
	bindUnimplemented( 0x016D, game_add_police, 4, "Add Police Restart" );
	bindFunction(0x016E, game_override_restart, 4, "Override Next Restart" );

	bindFunction(0x0180, game_link_mission_flag, 1, "Link ONMISSION Flag" );
	bindUnimplemented( 0x0181, game_link_character_mission_flag, 2, "Link Character Mission Flag" );
	bindUnimplemented( 0x0182, game_unknown, 2, "Unknown Character Opcode" );
	
	bindFunction(0x0186, game_add_object_blip<VehicleObject>, 2, "Add Blip for Vehicle");
	bindFunction(0x0187, game_add_object_blip<CharacterObject>, 2, "Add Blip for Character");

	bindFunction(0x018A, game_add_location_blip, 4, "Add Blip for Coord");
	bindFunction(0x018B, game_change_blip_mode, 2, "Change Blip Display Mode");
	bindUnimplemented( 0x018C, game_play_sound_at, 4, "Play Sound At" );
	bindUnimplemented( 0x018D, game_create_soundscape, 5, "Create soundscape" );

	bindUnimplemented( 0x018E, game_remove_sound, 1, "Remove Sound" );
	
	bindFunction(0x01B4, game_enable_input, 2, "Set Player Input Enabled" );

	bindFunction(0x01B6, game_set_weather, 1, "Set Weather Now" );
	bindUnimplemented(0x01B7, game_release_weather, 0, "Release Weather Lock" );
	
	bindFunction(0x01BD, game_get_runtime, 1, "Get Game Timer" );
	
	bindUnimplemented( 0x01C0, game_store_wanted_level, 2, "Store Wanted Level" );
	
	bindUnimplemented(0x01E0, game_clear_leader, 1, "Clear Leader" );
	
	bindFunction(0x01E3, game_print_big_with_number, 4, "Print Big With Number");
	
	bindFunction(0x01E7, game_disable_roads, 6, "Enable Roads" );
	bindFunction(0x01E8, game_enabled_roads, 6, "Disable Roads" );
	
	bindUnimplemented( 0x01EB, game_set_traffic_multiplier, 1, "Set Traffic Density Multiplier" );
	
	bindUnimplemented( 0x01ED, game_clear_threat_search, 1, "Clear Character Threat Search" );

	bindFunction(0x01F0, game_max_wanted_level, 1, "Set Max Wanted Level" );
	
	bindFunction(0x01F5, game_get_player, 2, "Get Player Character" );
	
	bindUnimplemented( 0x01F7, game_police_ignore_character, 2, "Set Cops Ignore Player" );
	bindFunction(0x01F6, game_clear_override, 0, "Clear override restart" );
	
	bindUnimplemented( 0x01F9, game_start_kill_frenzy, 9, "Start Kill Frenzy" );

	bindFunction(0x0219, game_create_garage, 8, "Create Garage" );

	bindUnimplemented( 0x021B, game_set_target_car_for_mission_garage, 2, "Set Target Car for Mission Garage" );
	bindFunction(0x021C, game_is_car_inside_garage, 1, "Is Car Inside Garage" );

	bindFunction(0x022A, game_disable_ped_paths, 6, "Disable ped paths" );
	bindFunction(0x022B, game_enable_ped_paths, 6, "Disable ped paths" );

	bindUnimplemented( 0x022D, game_character_face_player, 2, "Set Character Always Face Player" );
	
	bindUnimplemented( 0x022F, game_stop_character_look, 1, "Stop Character Looking" );

	bindUnimplemented( 0x0236, game_set_gang_car, 2, "Set Gang Car" );
	bindUnimplemented( 0x0237, game_set_gang_weapons, 3, "Set Gang Weapons" );

	bindFunction(0x023C, game_load_special_char, 2, "Load Special Character" );
	bindFunction(0x023D, game_special_char_loaded, 1, "Is Special Character Loaded" );

	bindFunction(0x0244, game_cutscene_offset, 3, "Set Cutscene Offset" );
	bindUnimplemented( 0x0245, game_set_character_anims, 2, "Set Character Animation Group" );

	bindUnimplemented( 0x0247, game_load_model, 1, "Request Model Loaded" );
	bindFunction(0x0248, game_model_loaded, 1, "Is Model Loaded" );
	bindUnimplemented( 0x0249, game_release_model, 1, "Mark Model As Unneeded" );
	bindUnimplemented( 0x024A, game_get_phone, 3, "Get Phone Near" );

	bindUnimplemented(0x024E, game_disable_phone, 1, "Turn phone off" );

	bindUnimplemented( 0x0250, game_create_light, 6, "Create Light" );
	
	bindFunction(0x0255, game_restart_critical_mission, 4, "Restart Critical Mission" );
	bindFunction(0x0256, game_is_player_playing, 1, "Is Player Playing" );

	bindFunction(0x0293, game_controller_mode, 1, "Get Controller Mode" );
	bindUnimplemented( 0x0294, game_set_car_resprayable, 2, "Set Car resprable" );
	bindUnimplemented( 0x0296, game_unload_special_characters, 1, "Unload Special Character" );
	
	bindUnimplemented( 0x0297, game_reset_kills, 0, "Reset Player Kills" );
	
	bindUnimplemented( 0x02A2, game_add_particle, 5, "Add Particle" );
	bindFunction(0x02A3, game_set_widescreen, 1, "Set Widescreen");
	
	bindFunction(0x02A7, game_add_contact_blip, 5, "Add Contact Blip");
	bindFunction(0x02A8, game_add_sprite_blip, 5, "Add Sprite Blip");
	
	bindFunction(0x02E4, game_load_cutscene, 1, "Load Cutscene Data" );
	bindFunction(0x02E5, game_create_cutscene_object, 2, "Create Cutscene Object" );
	bindFunction(0x02E6, game_set_cutscene_anim, 2, "Set Cutscene Animation" );
	bindFunction(0x02E7, game_start_cutscene, 0, "Start Cutscene" );
	bindFunction(0x02E8, game_get_cutscene_time, 1, "Get Cutscene Time" );
	bindFunction(0x02E9, game_cutscene_finished, 0, "Is Cutscene Over" );
	bindFunction(0x02EA, game_clear_cutscene, 0, "Clear Cutscene" );
	bindFunction(0x02EB, game_reset_camera, 0, "Reset Camera" );

	bindUnimplemented( 0x02EC, game_create_hidden_package, 3, "Create Hidden Package" );

	bindFunction(0x02ED, game_set_hidden_packages, 1, "Set Total Hidden Packages" );

	bindFunction(0x02F3, game_load_special_model, 2, "Load Special Model" );
	bindFunction(0x02F4, game_create_cutscene_head, 3, "Create Cutscene Actor Head" );
	bindFunction(0x02F5, game_set_head_animation, 2, "Set Cutscene Head Animation" );

	bindFunction(0x02FB, game_create_crusher_crane, 10, "Create Crusher Crane");

	bindFunction(0x030C, game_increment_progress, 1, "Increment Progress" );
	bindFunction(0x030D, game_set_max_progress, 1, "Set Max Progress" );

	bindFunction(0x0314, game_set_unique_jumps, 1, "Set Total Unique Jumps" );
	bindUnimplemented( 0x0315, game_increment_taxi_dropoffs, 0, "Increment Taxi Dropoffs" );
	bindUnimplemented( 0x0316, game_register_taxi_money, 1, "Register Taxi Money" );
	bindUnimplemented( 0x0317, game_increment_mission_attempts, 0, "Increment Mission Attempts" );
	bindFunction(0x0318, game_set_last_mission, 1, "Set Last completed mission" );
	
	bindFunction(0x0324, game_set_zone_ped_group, 3, "Set zone ped group" );
	bindUnimplemented( 0x0325, game_create_car_fire, 2, "Create Car Fire" );

	bindFunction( 0x0329, game_has_respray_happened, 1, "Has Respray Happened" );

	bindUnimplemented( 0x0335, game_free_resprays, 1, "Set Free Respray" );
	bindUnimplemented( 0x0336, game_set_character_visible, 2, "Set Player Visible" );

	bindFunction(0x033E, game_display_text, 3, "Display Text" );
	bindUnimplemented( 0x033F, game_set_text_colour, 2, "Set Text Scale" );
	bindFunction(0x0340, game_set_text_colour, 4, "Set Text Colour" );
	bindUnimplemented( 0x0341, game_set_text_justify, 1, "Set Text Justify" );
	bindUnimplemented( 0x0342, game_set_text_centered, 1, "Set Text Centered" );
	bindUnimplemented( 0x0344, game_set_center_text_size, 1, "Set Center Text Size" );
	bindUnimplemented( 0x0345, game_set_text_background, 1, "Set Text Background" );
	bindFunction(0x0346, game_set_background_colour, 4, "Set Text Background Colour" );

	bindUnimplemented( 0x0348, game_set_text_size_proportional, 1, "Set Text Size Proportional" );
	bindUnimplemented( 0x0349, game_set_font, 1, "Set Text Font" );
	
	bindFunction(0x0352, game_set_character_model, 2, "Set Character Model" );
	bindUnimplemented( 0x0353, game_refresh_character_model, 1, "Refresh Actor Model" );
	bindFunction( 0x0354, game_start_chase_scene, 1, "Start Chase Scene" );
	bindFunction( 0x0355, game_stop_chase_scene, 0, "Stop Chase Scene" );

	bindFunction(0x036D, game_print_big_with_2_numbers, 5, "Print Big With 2 Numbers");

	bindUnimplemented( 0x0373, game_camera_behind_player, 0, "Set Camera Behind Player" );
	bindUnimplemented( 0x0374, game_set_motion_blur, 1, "Set Motion Blur" );

	bindUnimplemented( 0x038B, game_load_models_now, 0, "Load Requested Models Now" );
	
	bindFunction(0x0394, game_play_music_id, 1, "Play music");
	bindFunction(0x0395, game_clear_area, 5, "Clear Area Vehicles and Pedestrians" );
	
	bindUnimplemented( 0x0397, game_set_vehicle_siren, 2, "Set Vehicle Siren" );
	
	bindUnimplemented( 0x0399, game_disable_ped_paths_in_area, 7, "Disable ped paths in angled cube" );

	bindUnimplemented( 0x039D, game_scatter_particles, 12, "Scatter Particles" );
	bindUnimplemented( 0x039E, game_set_character_hijackable, 2, "Set Character can be dragged out" );

	bindUnimplemented( 0x03AD, game_set_rubbish_enabled, 1, "Set Rubbish Enabled" );
	bindUnimplemented( 0x03AE, game_clear_area_particles, 6, "Remove Particles in Area" );
	bindUnimplemented( 0x03AF, game_set_streaming_enabled, 1, "Set Map Streaming Enabled" );

	bindUnimplemented( 0x03B7, game_custscen_only, 1, "Process Cutscene Only" );

	bindUnimplemented( 0x03BB, game_set_garage_rotating, 1, "Set Garage Door to Rotate" );

	bindUnimplemented( 0x03BF, game_set_peds_ignore_player, 2, "Set Pedestrians Ignoring Player" );
	
	bindUnimplemented( 0x03C1, game_store_character_vehicle_nosave, 2, "Store Player Vehicle No-Save" );
	
	bindUnimplemented( 0x03C4, game_display_counter_message, 3, "Display Counter Message" );
	bindUnimplemented( 0x03C5, game_create_vehicle_parked, 4, "Spawn Parked Vehicle" );
	bindFunction(0x03C6, game_collision_loaded, 1, "Is Collision In Memory" );
	
	bindUnimplemented( 0x03C8, game_camera_infront_player, 0, "Put Camera Infront of player" );
	
	bindUnimplemented( 0x03CB, game_load_area, 3, "Load Area Near" );
	
	bindFunction(0x03CF, game_load_audio, 1, "Load Audio" );
	
	bindFunction(0x03D0, game_is_audio_loaded, 0, "Is Audio Loaded" );
	bindFunction(0x03D1, game_play_mission_audio, 0, "Play Mission Audio" );
	bindFunction(0x03D2, game_is_audio_finished, 0, "Is Mission Audio Finished" );
	
	bindFunction(0x03D4, game_import_garage_contains_needed_car, 2, "Import Garage Contains Needed Car" );
	bindFunction(0x03D5, game_clear_print, 1, "Clear This Print" );
	bindUnimplemented( 0x03D6, game_clear_this_print, 1, "Clear This Big Print" );
	bindFunction(0x03D9, game_did_game_save, 0, "Did game save" );
	bindUnimplemented( 0x03DA, game_set_garage_follow_player, 1, "Set Garage Camera Follows Player" );
	
	bindFunction(0x03DC, game_add_object_blip<PickupObject>, 2, "Add blip for pickup");
	bindFunction(0x03DD, game_add_object_sprite_blip<PickupObject>, 3, "Add Sprite Blip for Pickup");
	
	bindUnimplemented( 0x03DE, game_set_pedestrian_density, 1, "Set Pedestrian density" );

	bindFunction(0x03E1, game_get_found_hidden_packages, 1, "Get Hidden Packages Found" );

	bindFunction(0x03E5, game_display_help, 1, "Display Help Text" );
	bindFunction(0x03E6, game_clear_help, 0, "Clear Help Text" );
	bindUnimplemented( 0x03E7, game_flash_hud, 1, "Flash HUD Item" );
	
	bindUnimplemented( 0x03EB, game_clear_prints, 0, "Clear Small Prints" );
	bindFunction(0x03EC, game_has_crane_collected_all_cars, 0, "Has Crane Collected All Cars" );

	bindFunction(0x03EE, game_can_player_move, 1, "Can Player Move" );
	bindUnimplemented( 0x03EF, game_safe_player, 1, "Make Player Safe For Cutscene" );

	bindUnimplemented( 0x03F0, game_enabled_text, 1, "Enable Text Draw" );
	bindUnimplemented( 0x03F1, game_set_character_hostility, 2, "Set Ped Hostility" );
	bindUnimplemented( 0x03F2, game_clear_character_hostility, 2, "Clear Ped Hostility" );

	bindFunction(0x03F7, game_load_collision, 1, "Load Collision" );

	bindUnimplemented(0x0405, game_enable_phone, 1, "Turn on phone" );

	bindFunction(0x0408, game_set_rampages, 1, "Set Total Rampage Missions" );
	bindUnimplemented( 0x0409, game_explode_rc_buggy, 0, "Blow up RC buggy" );
	bindFunction( 0x040A, game_remove_chase_car, 1, "Remove Chase Car" );

	bindUnimplemented( 0x0418, game_set_object_ontop, 2, "Set Object Draw Ontop" );

	bindFunction(0x041D, game_set_near_clip, 1, "Set Camera Near Clip" );
	bindUnimplemented( 0x041E, game_set_radio, 2, "Set Radio Station" );

	bindUnimplemented( 0x0421, game_force_rain, 1, "Force Rain" );
	bindFunction( 0x0422, game_garage_contains_car, 2, "Garage Contains Car" );

	bindUnimplemented( 0x0426, game_create_level_transition, 6, "Create Save Cars Between Levels cube" );
	
	bindFunction(0x042C, game_set_missions, 1, "Set Total Missions" );

	bindFunction(0x043C, game_set_sound_fade, 1, "Set Sound Fade" );
	bindFunction(0x043D, game_set_is_intro_playing, 1, "Set Is Intro Playing" );

	bindFunction(0x0445, game_are_vehicle_cheats_enabled, 0, "Are Any Vehicle Cheats enabled" );

	bindFunction(0x044D, game_load_splash, 1, "Load Splash Screen" );
	
	bindUnimplemented( 0x0452, game_enable_user_camera, 0, "Enable User Camera Controll" );
}
