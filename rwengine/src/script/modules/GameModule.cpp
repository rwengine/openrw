#include "script/modules/GameModule.hpp"
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameWorld.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

void game_print_big(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getVM()->getWorld()->gameData.texts.text(id);
	unsigned short style = args[2].integer;
	args.getVM()->getWorld()->state.text.push_back({
		id,
		str,
		args.getVM()->getWorld()->gameTime,
		args[1].integer / 1000.f,
		style
	});
}

void game_print_now(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getVM()->getWorld()->gameData.texts.text(id);
	int flags = args[2].integer;
	args.getVM()->getWorld()->state.text.push_back({
		id,
		str,
		args.getVM()->getWorld()->gameTime,
		args[1].integer / 1000.f,
		0
	});
}

void game_clear_prints(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.text.clear();
}

void game_get_time(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getVM()->getWorld()->getHour();
	*args[1].globalInteger = args.getVM()->getWorld()->getMinute();
}

void game_set_time(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.hour = args[0].integer;
	args.getVM()->getWorld()->state.minute = args[1].integer;
}

bool game_is_button_pressed(const ScriptArguments& args)
{
	/// @todo implement
	return false;
}

void game_set_dead_or_arrested(const ScriptArguments& args)
{
	*args.getVM()->getWorld()->state.scriptOnMissionFlag = args[0].integer;
}
bool game_has_death_or_arrest_finished(const ScriptArguments& args)
{
	return false;
}

void game_create_vehicle_generator(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	
	if(args[4].type == TString)
	{
		std::cerr << "Model names not supported for vehicle generators" << std::endl;
		return;
	}
	
	VehicleGenerator vg;
	vg.position = position;
	vg.heading = args[3].real;
	vg.vehicleID = args[4].integer;
	vg.colourFG = args[5].integer;
	vg.colourBG = args[6].integer;
	vg.alwaysSpawn = args[7].integer != 0;
	vg.alarmThreshold = args[8].integer;
	vg.lockedThreshold = args[9].integer;
	vg.minDelay = args[10].integer;
	vg.maxDelay = args[11].integer;
	
	vg.lastSpawnTime = 0;
	vg.remainingSpawns = 0;
	
	*args[12].globalInteger = args.getVM()->getWorld()->state.vehicleGenerators.size();
	
	args.getVM()->getWorld()->state.vehicleGenerators.push_back(vg);
}
void game_set_vehicle_generator_count(const ScriptArguments& args)
{
	VehicleGenerator& generator = args.getVM()->getWorld()->state.vehicleGenerators.at(*args[0].globalInteger);
	generator.remainingSpawns = args[1].integer;
}

void game_set_zone_car_info(const ScriptArguments& args)
{
	auto it = args.getVM()->getWorld()->gameData.zones.find(args[0].string);
	if( it != args.getVM()->getWorld()->gameData.zones.end() )
	{
		auto day = args[1].integer == 1;
		for(int i = 2; i < args.getParameters().size(); ++i)
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
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	if( controller != nullptr )
	{
		args.getVM()->getWorld()->state.cameraTarget = controller->getCharacter();
	}
}

void game_reset_camera(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.cameraTarget = nullptr;
	args.getVM()->getWorld()->state.cameraFixed = false;
}

void game_set_zone_ped_info(const ScriptArguments& args)
{
	auto it = args.getVM()->getWorld()->gameData.zones.find(args[0].string);
	if( it != args.getVM()->getWorld()->gameData.zones.end() )
	{
		auto day = args[1].integer == 1;
		for(int i = 2; i < args.getParameters().size(); ++i)
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
	
	args.getVM()->getWorld()->state.cameraFixed = true;
	args.getVM()->getWorld()->state.cameraPosition = position;
	args.getVM()->getWorld()->state.cameraRotation = glm::quat(angles);
}
void game_camera_lookat_position(const ScriptArguments& args)
{
	glm::vec3 position( args[0].real, args[1].real, args[2].real );
	int switchmode = args[3].integer;
	
	auto direction = glm::normalize(position - args.getVM()->getWorld()->state.cameraPosition);
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
	
	args.getVM()->getWorld()->state.cameraRotation = glm::inverse(glm::quat_cast(v));
}

void game_set_fade_colour(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.fadeColour.r = args[0].integer;
	args.getVM()->getWorld()->state.fadeColour.g = args[1].integer;
	args.getVM()->getWorld()->state.fadeColour.b = args[2].integer;
}
void game_fade_screen(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.fadeTime = args[0].integer / 1000.f;
	args.getVM()->getWorld()->state.fadeOut = !!args[1].integer;
	args.getVM()->getWorld()->state.fadeStart = args.getVM()->getWorld()->gameTime;
}
bool game_screen_fading(const ScriptArguments& args)
{
	if( args.getVM()->getWorld()->state.skipCutscene )
	{
		return false;
	}
	return args.getVM()->getWorld()->gameTime <
		args.getVM()->getWorld()->state.fadeStart + args.getVM()->getWorld()->state.fadeTime;
}

void game_override_restart(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.overrideNextStart = true;
	args.getVM()->getWorld()->state.nextRestartLocation = glm::vec4(
		args[0].real, args[1].real, args[2].real, args[3].real
	);
}

void game_clear_override(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.overrideNextStart = false;
}

void game_link_mission_flag(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.scriptOnMissionFlag = (unsigned int*)args[0].globalInteger;
}

void game_enable_input(const ScriptArguments& args)
{
	auto controller = static_cast<PlayerController*>(*args[0].handle);
	controller->setInputEnabled(!!args[1].integer);
}

void game_set_weather(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.currentWeather = args[0].integer;
}

void game_get_runtime(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getVM()->getWorld()->gameTime * 1000;
}

void game_disable_roads(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getVM()->getWorld()->enableAIPaths(AIGraphNode::Vehicle, min, max);
}
void game_enabled_roads(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getVM()->getWorld()->disableAIPaths(AIGraphNode::Vehicle, min, max);
}

void game_max_wanted_level(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.maxWantedLevel = args[0].integer;
}

// This does nothing for us.
void game_get_player(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	*args[1].handle = controller;
}

bool game_is_pickup_collected(const ScriptArguments& args)
{
	/// @todo 
	return false;
}

void game_create_garage(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real, args[1].real, args[2].real);
	glm::vec3 max(args[3].real, args[4].real, args[5].real);

	/// @todo http://www.gtamodding.com/index.php?title=Garage#GTA_III
	int garageType = args[6].integer;
	auto garageHandle = args[7].handle;

	std::cout << "Garages Unimplemented. type " << garageType << std::endl;
}

void game_disable_ped_paths(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getVM()->getWorld()->enableAIPaths(AIGraphNode::Pedestrian, min, max);
}
void game_enable_ped_paths(const ScriptArguments& args)
{
	glm::vec3 min(args[0].real,args[1].real,args[2].real);
	glm::vec3 max(args[3].real,args[4].real,args[5].real);
	
	args.getVM()->getWorld()->disableAIPaths(AIGraphNode::Pedestrian, min, max);
}

void game_load_special_char(const ScriptArguments& args)
{
	args.getVM()->getWorld()->loadSpecialCharacter(args[0].integer, args[1].string);
}
bool game_special_char_loaded(const ScriptArguments& args)
{
	auto chartype = args.getVM()->getWorld()->findObjectType<CharacterData>(args[0].integer);
	if( chartype ) {
		auto modelfind = args.getVM()->getWorld()->gameData.models.find(chartype->modelName);
		if( modelfind != args.getVM()->getWorld()->gameData.models.end() && modelfind->second->model != nullptr ) {
			return true;
		}
	}

	return true;
}

void game_cutscene_offset(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	if( args.getVM()->getWorld()->state.currentCutscene ) {
		args.getVM()->getWorld()->state.currentCutscene->meta.sceneOffset = position;
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
	std::cout << "Restting player" << std::endl;
	// Reset player state.
	glm::vec3 position(args[0].real, args[1].real, args[2].real + 1.f);
	
	auto controller = args.getVM()->getWorld()->state.player;
	
	glm::vec3 spawnMagic( 0.f, 0.f, 1.f );
	
	controller->getCharacter()->setPosition(position + spawnMagic);
	
	controller->getCharacter()->setHeading( args[3].real );
	
	/// @todo find a nicer way to implement warping out of vehicles.
	auto cv = controller->getCharacter()->getCurrentVehicle();
	if ( cv != nullptr )
	{
		cv->setOccupant( controller->getCharacter()->getCurrentSeat(), nullptr );
		controller->getCharacter()->setCurrentVehicle(nullptr, 0);
	}
}

/// @todo http://www.gtamodding.com/index.php?title=0256 (e.g. check if dead or busted)
bool game_is_player_playing(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	return controller != nullptr;
}

void game_controller_mode(const ScriptArguments& args)
{
	*args[0].globalInteger	= 0;
}

void game_set_widescreen(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.isCinematic = !!args[0].integer;
}

void game_load_cutscene(const ScriptArguments& args)
{
	args.getVM()->getWorld()->loadCutscene(args[0].string);
	args.getVM()->getWorld()->state.cutsceneStartTime = -1.f;
}
void game_create_cutscene_object(const ScriptArguments& args)
{
	auto id	= args[0].integer;

	GameObject* object = object = args.getVM()->getWorld()->createCutsceneObject(id, args.getVM()->getWorld()->state.currentCutscene->meta.sceneOffset );
	*args[1].handle = object;

	if( object == nullptr ) {
		std::cerr << "Could not create cutscene object " << id << std::endl;
	}
}
void game_set_cutscene_anim(const ScriptArguments& args)
{
	GameObject* object = static_cast<GameObject*>(*args[0].handle);
	std::string animName = args[1].string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = args.getVM()->getWorld()->gameData.animations[animName];
	if( anim ) {
		object->animator->setAnimation(anim, false);
	}
	else {
		std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
	}
}
void game_start_cutscene(const ScriptArguments& args)
{
	args.getVM()->getWorld()->startCutscene();
}
void game_get_cutscene_time(const ScriptArguments& args)
{
	float time = args.getVM()->getWorld()->gameTime - args.getVM()->getWorld()->state.cutsceneStartTime;
	if( args.getVM()->getWorld()->state.skipCutscene )
	{
		*args[0].globalInteger = args.getVM()->getWorld()->state.currentCutscene->tracks.duration * 1000;
	}
	else
	{
		*args[0].globalInteger = time * 1000;
	}
}
bool game_cutscene_finished(const ScriptArguments& args)
{
	if( args.getVM()->getWorld()->state.currentCutscene ) {
		float time = args.getVM()->getWorld()->gameTime - args.getVM()->getWorld()->state.cutsceneStartTime;
		if( args.getVM()->getWorld()->state.skipCutscene ) {
			return true;
		}
		return time > args.getVM()->getWorld()->state.currentCutscene->tracks.duration;
	}
	return true;
}
void game_clear_cutscene(const ScriptArguments& args)
{
	args.getVM()->getWorld()->clearCutscene();
}

void game_set_hidden_packages(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.numHiddenPackages = args[0].integer;
}

void game_load_special_model(const ScriptArguments& args)
{
	args.getVM()->getWorld()->loadSpecialModel(args[0].integer, args[1].string);
}
void game_create_cutscene_head(const ScriptArguments& args)
{
	auto id = args[1].integer;
	auto actor = static_cast<GameObject*>(*args[0].handle);
	CutsceneObject* object = args.getVM()->getWorld()->createCutsceneObject(id, args.getVM()->getWorld()->state.currentCutscene->meta.sceneOffset );

	auto headframe = actor->model->model->findFrame("shead");
	actor->skeleton->setEnabled(headframe, false);
	object->setParentActor(actor, headframe);

	*args[2].handle = object;
}
void game_set_head_animation(const ScriptArguments& args)
{
	GameObject* object = static_cast<GameObject*>(*args[0].handle);
	std::string animName = args[1].string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = args.getVM()->getWorld()->gameData.animations[animName];
	if( anim ) {
		object->animator->setAnimation(anim, false);
	}
	else {
		std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
	}
}

void game_increment_progress(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.currentProgress += args[0].integer;
}

void game_set_max_progress(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.maxProgress = args[0].integer;
}

void game_set_unique_jumps(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.numUniqueJumps = args[0].integer;
}

void game_set_last_mission(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.lastMissionName = args[0].string;
}

void game_set_zone_ped_group(const ScriptArguments& args)
{
	auto it = args.getVM()->getWorld()->gameData.zones.find(args[0].string);
	if( it != args.getVM()->getWorld()->gameData.zones.end() )
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

void game_display_text(const ScriptArguments& args)
{
	glm::vec2 pos(args[0].real, args[1].real);
	std::string str(args[2].string);
	str = args.getVM()->getWorld()->gameData.texts.text(str);
	args.getVM()->getWorld()->state.nextText.text = str;
	args.getVM()->getWorld()->state.nextText.position = pos;
	args.getVM()->getWorld()->state.texts.push_back(args.getVM()->getWorld()->state.nextText);
}

void game_set_text_colour(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.nextText.colourFG.r = args[0].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourFG.g = args[1].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourFG.b = args[2].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourFG.a = args[3].integer / 255.f;
}

void game_set_background_colour(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.nextText.colourBG.r = args[0].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourBG.g = args[1].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourBG.b = args[2].integer / 255.f;
	args.getVM()->getWorld()->state.nextText.colourBG.a = args[3].integer / 255.f;
}

void game_set_character_model(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	controller->getCharacter()->changeCharacterModel(args[1].string);
}

bool game_collision_loaded(const ScriptArguments& args)
{
	// The paramter to this is actually the island number.
	// Not sure how that will fit into the scheme of full paging
	/// @todo use the current player zone island number to return the correct value.
	return true;
}

void game_load_audio(const ScriptArguments& args)
{
	std::string name = args[0].string;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if(! args.getVM()->getWorld()->gameData.loadAudio(args.getVM()->getWorld()->missionAudio, name + ".wav"))
	{
		std::cerr << "Couldn't load mission audio " << name << std::endl;
	}
}
bool game_is_audio_loaded(const ScriptArguments& args)
{
	return true;
}
void game_play_mission_audio(const ScriptArguments& args)
{
	args.getVM()->getWorld()->missionSound.setBuffer(args.getVM()->getWorld()->missionAudio);
	args.getVM()->getWorld()->missionSound.play();
	args.getVM()->getWorld()->missionSound.setLoop(false);
}
bool game_is_audio_finished(const ScriptArguments& args)
{
	return args.getVM()->getWorld()->missionSound.getStatus() == sf::SoundSource::Stopped;
}

void game_play_music_id(const ScriptArguments& args)
{
	int id = args[0].integer;
	GameWorld* gw = args.getVM()->getWorld();
	std::string name = "Miscom";
	
	// TODO play anything other than Miscom.wav
	if(! gw->gameData.loadAudio(args.getVM()->getWorld()->missionAudio, name + ".wav") )
	{
		std::cerr << "Error loading audio" << std::endl;
		return;
	}
	
	gw->missionSound.setBuffer(args.getVM()->getWorld()->missionAudio);
	gw->missionSound.play();
	gw->missionSound.setLoop(false);
}

void game_clear_print(const ScriptArguments& args)
{
	std::string id(args[0].string);
	
	for( int i = 0; i < args.getVM()->getWorld()->state.text.size(); )
	{
		if( args.getVM()->getWorld()->state.text[i].id == id )
		{
			args.getVM()->getWorld()->state.text.erase(args.getVM()->getWorld()->state.text.begin() + i);
		}
		else
		{
			i++;
		}
	}
}

void game_get_found_hidden_packages(const ScriptArguments& args)
{
	*args[0].globalInteger = args.getVM()->getWorld()->state.numHiddenPackagesDiscovered;
}

void game_display_help(const ScriptArguments& args)
{
	std::string id(args[0].string);
	std::string str = args.getVM()->getWorld()->gameData.texts.text(id);
	unsigned short style = 12;
	args.getVM()->getWorld()->state.text.push_back({
		id,
		str,
		args.getVM()->getWorld()->gameTime,
		2.5f,
		style
	});
}

bool game_can_player_move(const ScriptArguments& args)
{
	return true;
}

void game_load_collision(const ScriptArguments& args)
{
	// Collision is loaded when required, not sure what this is supposed to mean.
}

void game_set_rampages(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.numRampages = args[0].integer;
}

void game_set_near_clip(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.cameraNear = args[0].real;
}

void game_set_missions(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.numMissions = args[0].integer;
}

void game_set_sound_fade(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.fadeSound = !!args[0].integer;
}
void game_set_is_intro_playing(const ScriptArguments& args)
{
	args.getVM()->getWorld()->state.isIntroPlaying = !!args[0].integer;
}

bool game_are_vehicle_cheats_enabled(const ScriptArguments& args)
{
	return false;
}

void game_load_splash(const ScriptArguments& args)
{
	args.getVM()->getWorld()->gameData.loadSplash(args[0].string);
}

GameModule::GameModule()
: ScriptModule("Game")
{
	bindUnimplemented(0x00A7, game_drive_to, 4, "Drive To" );
	
	bindUnimplemented( 0x00AD, game_set_driving_speed, 2, "Set Driving Speed" );
	bindUnimplemented( 0x00AE, game_set_driving_style, 2, "Set Driving Style" );
	
	bindFunction(0x00BA, game_print_big, 3, "Print big" );
	bindFunction(0x00BC, game_print_now, 3, "Print Message Now" );
	
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
	
	bindUnimplemented( 0x014E, game_show_onscreen_timer, 1, "Display Onscreen Timer" );
	
	bindUnimplemented( 0x014F, game_stop_timer, 1, "Stop Timer" );
	
	bindUnimplemented( 0x0151, game_clear_counter, 1, "Clear Counter" );
	bindFunction(0x0152, game_set_zone_car_info, 17, "Set zone car info" );
	
	bindUnimplemented( 0x0158, game_camera_follow_vehicle, 3, "Camera Follow Vehicle" );
	bindFunction(0x0159, game_camera_follow_character, 3, "Camera Follow Character" );
	
	bindFunction(0x015A, game_reset_camera, 0, "Reset Camera" );
	
	bindFunction(0x015C, game_set_zone_ped_info, 11, "Set zone ped info" );

	bindFunction(0x015F, game_camera_fixed_position, 6, "Set Fixed Camera Position" );
	bindFunction(0x0160, game_camera_lookat_position, 4, "Point Camera at Point" );
	
	bindUnimplemented( 0x0164, game_disable_blip, 1, "Disable Radar Blip" );

	bindFunction(0x0169, game_set_fade_colour, 3, "Set Fade Colour" );
	bindFunction(0x016A, game_fade_screen, 2, "Fade Screen" );
	bindFunction(0x016B, game_screen_fading, 0, "Is Screen Fading" );
	bindUnimplemented( 0x016C, game_add_hopsital, 4, "Add Hospital Restart" );
	bindUnimplemented( 0x016D, game_add_police, 4, "Add Police Restart" );
	bindFunction(0x016E, game_override_restart, 4, "Override Next Restart" );

	bindFunction(0x0180, game_link_mission_flag, 1, "Link ONMISSION Flag" );
	bindUnimplemented( 0x0181, game_link_character_mission_flag, 2, "Link Character Mission Flag" );
	bindUnimplemented( 0x0182, game_unknown, 2, "Unknown Character Opcode" );
	
	bindUnimplemented( 0x0186, game_add_vehicle_blip, 2, "Add Blip for Vehicle" );
	bindUnimplemented( 0x0187, game_add_character_blip, 2, "Add Blip for Character" );

	bindUnimplemented( 0x018A, game_add_location_blip, 4, "Add Blip for Coord" );
	bindUnimplemented( 0x018B, game_change_blip_mode, 2, "Change Blip Display Mode" );
	
	bindUnimplemented( 0x018D, game_create_soundscape, 5, "Create soundscape" );

	bindUnimplemented( 0x018E, game_remove_sound, 1, "Remove Sound" );
	
	bindFunction(0x01B4, game_enable_input, 2, "Set Player Input Enabled" );

	bindFunction(0x01B6, game_set_weather, 1, "Set Weather Now" );
	bindUnimplemented(0x01B7, game_release_weather, 0, "Release Weather Lock" );
	
	bindFunction(0x01BD, game_get_runtime, 1, "Get Game Timer" );
	
	bindUnimplemented( 0x01C0, game_store_wanted_level, 2, "Store Wanted Level" );
	
	bindUnimplemented(0x01E0, game_clear_leader, 1, "Clear Leader" );
	
	bindUnimplemented(0x01E3, game_print_big_with_number, 4, "Print Big With Number" );
	
	bindFunction(0x01E7, game_disable_roads, 6, "Enable Roads" );
	bindFunction(0x01E8, game_enabled_roads, 6, "Disable Roads" );
	
	bindUnimplemented( 0x01EB, game_set_traffic_multiplier, 1, "Set Traffic Density Multiplier" );
	
	bindUnimplemented( 0x01ED, game_clear_threat_search, 1, "Clear Character Threat Search" );

	bindFunction(0x01F0, game_max_wanted_level, 1, "Set Max Wanted Level" );
	
	bindFunction(0x01F5, game_get_player, 2, "Get Player Character" );
	
	bindUnimplemented( 0x01F7, game_police_ignore_character, 2, "Set Cops Ignore Player" );
	bindFunction(0x01F6, game_clear_override, 0, "Clear override restart" );
	
	bindUnimplemented( 0x01F9, game_start_kill_frenzy, 9, "Start Kill Frenzy" );
	
	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0213/
	bindUnimplemented( 0x0213, game_create_pickup, 6, "Create pickup" );
	bindFunction(0x0214, game_is_pickup_collected, 1, "Has Pickup been collected" );
	bindUnimplemented( 0x0215, game_destroy_pickup, 1, "Destroy Pickup" );

	bindFunction(0x0219, game_create_garage, 8, "Create Garage" );

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

	bindUnimplemented( 0x024A, game_get_phone, 3, "Get Phone Near" );

	bindFunction(0x0248, game_model_loaded, 1, "Is Model Loaded" );
	bindUnimplemented( 0x0249, game_release_model, 1, "Mark Model As Unneeded" );

	bindUnimplemented( 0x0250, game_create_light, 6, "Create Light" );
	
	bindFunction(0x0255, game_restart_critical_mission, 4, "Restart Critical Mission" );
	bindFunction(0x0256, game_is_player_playing, 1, "Is Player Playing" );

	bindFunction(0x0293, game_controller_mode, 1, "Get Controller Mode" );

	bindUnimplemented( 0x0296, game_unload_special_characters, 1, "Unload Special Character" );
	
	bindUnimplemented( 0x0297, game_reset_kills, 0, "Reset Player Kills" );
	
	bindUnimplemented( 0x02A2, game_add_particle, 5, "Add Particle" );
	bindFunction(0x02A3, game_set_widescreen, 1, "Set Widescreen" );
	
	bindUnimplemented( 0x02A7, game_add_contact_blip, 5, "Add Radar Contact Blip" );
	bindUnimplemented( 0x02A8, game_add_radar_blip, 5, "Add Radar Blip" );
	
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

	bindFunction(0x030C, game_increment_progress, 1, "Increment Progress" );
	bindFunction(0x030D, game_set_max_progress, 1, "Set Max Progress" );

	bindFunction(0x0314, game_set_unique_jumps, 1, "Set Total Unique Jumps" );
	
	bindUnimplemented( 0x0317, game_increment_mission_attempts, 0, "Increment Mission Attempts" );
	bindFunction(0x0318, game_set_last_mission, 1, "Set Last completed mission" );
	
	bindFunction(0x0324, game_set_zone_ped_group, 3, "Set zone ped group" );
	bindUnimplemented( 0x0325, game_create_car_fire, 2, "Create Car Fire" );

	bindUnimplemented( 0x032B, game_create_weapon_pickup, 7, "Create Weapon Pickup" );

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
	bindUnimplemented( 0x0354, game_start_chase, 1, "Start Chase Scene" );
	bindUnimplemented( 0x0355, game_stop_chase, 0, "Stop Chase Scene" );

	bindUnimplemented( 0x0373, game_camera_behind_player, 0, "Set Camera Behind Player" );
	bindUnimplemented( 0x0374, game_set_motion_blur, 1, "Set Motion Blur" );

	bindUnimplemented( 0x038B, game_load_models_now, 0, "Load Requested Models Now" );
	
	bindFunction(0x0394, game_play_music_id, 1, "Play music");
	bindUnimplemented( 0x0395, game_clear_area, 5, "Clear Area Vehicles and Pedestrians" );
	
	bindUnimplemented( 0x0397, game_set_vehicle_siren, 2, "Set Vehicle Siren" );
	
	bindUnimplemented( 0x0399, game_disable_ped_paths_in_area, 7, "Disable ped paths in angled cube" );

	bindUnimplemented( 0x039D, game_scatter_particles, 12, "Scatter Particles" );
	bindUnimplemented( 0x039E, game_set_character_hijackable, 2, "Set Character can be dragged out" );

	bindUnimplemented( 0x03AD, game_set_garage_enabled, 1, "Set Garbage Enabled" );
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
	
	bindFunction(0x03D5, game_clear_print, 1, "Clear This Print" );
	bindUnimplemented( 0x03D6, game_clear_this_print, 1, "Clear This Big Print" );

	bindUnimplemented( 0x03DA, game_set_garage_follow_player, 1, "Set Garage Camera Follows Player" );
	
	bindUnimplemented( 0x03DE, game_set_pedestrian_density, 1, "Set Pedestrian density" );

	bindFunction(0x03E1, game_get_found_hidden_packages, 1, "Get Hidden Packages Found" );

	bindFunction(0x03E5, game_display_help, 1, "Display Help Text" );
	bindUnimplemented( 0x03E6, game_clear_help, 0, "Clear Help Text" );
	bindUnimplemented( 0x03E7, game_flash_hud, 1, "Flash HUD Item" );
	
	bindUnimplemented( 0x03EB, game_clear_prints, 0, "Clear Small Prints" );

	bindFunction(0x03EE, game_can_player_move, 1, "Can Player Move" );
	bindUnimplemented( 0x03EF, game_safe_player, 1, "Make Player Safe For Cutscene" );

	bindUnimplemented( 0x03F0, game_enabled_text, 1, "Enable Text Draw" );
	bindUnimplemented( 0x03F1, game_set_character_hostility, 2, "Set Ped Hostility" );
	bindUnimplemented( 0x03F2, game_clear_character_hostility, 2, "Clear Ped Hostility" );

	bindFunction(0x03F7, game_load_collision, 1, "Load Collision" );

	bindFunction(0x0408, game_set_rampages, 1, "Set Total Rampage Missions" );
	bindUnimplemented( 0x0409, game_explode_rc_buggy, 0, "Blow up RC buggy" );
	bindUnimplemented( 0x040A, game_remove_chase_car, 1, "Remove Chase Car" );

	bindUnimplemented( 0x0418, game_set_object_ontop, 2, "Set Object Draw Ontop" );

	bindFunction(0x041D, game_set_near_clip, 1, "Set Camera Near Clip" );
	bindUnimplemented( 0x041E, game_set_radio, 2, "Set Radio Station" );

	bindUnimplemented( 0x0421, game_force_rain, 1, "Force Rain" );

	bindUnimplemented( 0x0426, game_create_level_transition, 6, "Create Save Cars Between Levels cube" );
	
	bindFunction(0x042C, game_set_missions, 1, "Set Total Missions" );

	bindFunction(0x043C, game_set_sound_fade, 1, "Set Sound Fade" );
	bindFunction(0x043D, game_set_is_intro_playing, 1, "Set Is Intro Playing" );

	bindFunction(0x0445, game_are_vehicle_cheats_enabled, 0, "Are Any Vehicle Cheats enabled" );

	bindFunction(0x044D, game_load_splash, 1, "Load Splash Screen" );
	
	bindUnimplemented( 0x0452, game_enable_user_camera, 0, "Enable User Camera Controll" );
}
