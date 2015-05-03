#include <engine/SaveGame.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/GameObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>

#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>

namespace cereal
{

template<class Archive>
void serialize(Archive& archive, 
          glm::vec3& s)
{ 
	archive(s.x, s.y, s.z);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::vec4& s)
{ 
	archive(s.x, s.y, s.z, s.w);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::u16vec3& s)
{ 
	archive(s.x, s.y, s.z);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::quat& s)
{ 
	archive(s.x, s.y, s.z, s.w);
}

template<class Archive>
void serialize(Archive& archive, 
          OnscreenText& t)
{ 
	archive(
		t.id,
		t.osTextString,
		t.osTextStart,
		t.osTextTime,
		t.osTextStyle);
}

template<class Archive>
void serialize(Archive& archive, 
          VehicleGenerator& s)
{ 
	archive(
		s.position,
		s.heading,
		s.vehicleID,
		s.colourFG,
		s.colourBG,
		s.alwaysSpawn,
		s.alarmThreshold,
		s.lockedThreshold,
		s.minDelay,
		s.maxDelay,
		s.lastSpawnTime,
		s.remainingSpawns);
}

template<class Archive>
void serialize(Archive& archive, 
          BlipData& s)
{ 
	archive(
		s.id,
		s.target,
		s.coord,
		s.texture,
		s.display);
}

template<class Archive>
void serialize(Archive& archive, 
          GameState& s)
{ 
	archive(
		s.gameTime,
		s.currentProgress,
		s.maxProgress,
		s.numMissions,
		s.numHiddenPackages,
		s.numHiddenPackagesDiscovered,
		s.numUniqueJumps,
		s.numRampages,
		s.maxWantedLevel,
		s.playerObject,
		s.currentWeather,
		s.missionObjects,
		s.overrideNextStart,
		s.nextRestartLocation,
		s.fadeOut,
		s.fadeStart,
		s.fadeTime,
		s.fadeSound,
		s.fadeColour,
		s.currentSplash,
		s.skipCutscene,
		s.isIntroPlaying,
		s.isCinematic,
		s.hour,
		s.minute,
		s.lastMissionName,
		s.specialCharacters,
		s.specialModels,
		s.text,
		s.cameraNear,
		s.cameraFixed,
		s.cameraPosition,
		s.cameraRotation,
		s.cameraTarget,
		s.vehicleGenerators,
		s.radarBlips); 
}

template<class Archive>
void serialize(Archive& archive, 
          SCMThread& s)
{ 
	archive(
		s.name,
		s.baseAddress,
		s.programCounter,
		s.conditionCount,
		s.conditionResult,
		s.conditionMask,
		s.conditionAND,
		s.wakeCounter,
		s.locals,
		s.isMission,
		s.finished,
		s.stackDepth,
		s.calls);
}

template<class Archive>
void serialize(Archive& archive, 
          ScriptMachine& s)
{ 
	archive(
		s.getThreads(),
		s.getGlobalData());
}
}

void SaveGame::writeState(GameState& state, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		oa(state);
	}
}

bool SaveGame::loadState(GameState& state, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		ia(state);
	}

	return true;
}

void SaveGame::writeScript(ScriptMachine& sm, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		oa(sm);
	}
}

bool SaveGame::loadScript(ScriptMachine& sm, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		ia(sm);
	}

	return true;
}

void SaveGame::writeObjects(GameWorld& world, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		std::vector<GameObject*> writeable;
		for( auto& p : world.objects )
		{
			switch( p.second->type() )
			{
				case GameObject::Vehicle:
				case GameObject::Character:
					break;
				default:
					continue;
			}
			if( p.second->getLifetime() == GameObject::TrafficLifetime )
			{
				continue;
			}
			writeable.push_back(p.second);
		}

		// Write object count.
		oa(writeable.size());
		for( GameObject* saved : writeable )
		{
			oa(saved->getGameObjectID());
			oa(saved->type());
			oa(saved->getLifetime());
			oa(saved->getPosition());
			oa(saved->getRotation());
			switch( saved->type() )
			{
			case GameObject::Vehicle:
			{
				auto vehicle = static_cast<VehicleObject*>(saved);
				oa(vehicle->vehicle->ID);
			} break;
			case GameObject::Character:
			{
				auto character = static_cast<CharacterObject*>(saved);
				oa(character->ped->ID);
			} break;
			}
		}
	}
}


bool SaveGame::loadObjects(GameWorld& world, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		std::size_t num;
		ia(num);
		for(int i = 0; i < num; i++)
		{
			GameObjectID gameID;
			GameObject::Type type;
			GameObject::ObjectLifetime lifetime;
			glm::vec3 translation;
			glm::quat orientation;

			ia(gameID);
			ia(type);
			ia(lifetime);
			ia(translation);
			ia(orientation);

			switch( type )
			{
			case GameObject::Vehicle:
			{
				ObjectID id;
				ia(id);
				auto vehicle = world.createVehicle(id, translation, orientation, gameID);
				vehicle->setLifetime(lifetime);
			} break;
			case GameObject::Character:
			{
				ObjectID id;
				ia(id);
				CharacterObject* character;
				if( lifetime == GameObject::PlayerLifetime )
				{
					character = world.createPlayer(translation, orientation, gameID);
				}
				else
				{
					character = world.createPedestrian(id, translation, orientation, gameID);
				}
			} break;
			}
		}
	}

	return true;
}
