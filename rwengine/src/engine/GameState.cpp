#include <engine/GameState.hpp>

GameState::GameState() :
currentProgress(0),
maxProgress(1),
numMissions(0),
numHiddenPackages(0),
numHiddenPackagesDiscovered(0),
numUniqueJumps(0),
numRampages(0),
maxWantedLevel(0),
player(nullptr),
currentWeather(0),
scriptOnMissionFlag(nullptr),
fadeOut(true),
fadeStart(0.f),
fadeTime(0.f),
fadeSound(false),
skipCutscene(false),
isIntroPlaying(false),
currentCutscene(nullptr),
cutsceneStartTime(-1.f),
isCinematic(false),
hour(0),
minute(0),
cameraNear(0.1f),
cameraFixed(false),
cameraTarget(nullptr),
world(nullptr)
{
	
}

int GameState::addRadarBlip(BlipData& blip)
{
	int l = 0;
	for ( auto x = radarBlips.begin(); x != radarBlips.end(); ++x )
	{
		if ( (x->first) != l )
		{
			l = x->first-1;
		}
		else
		{
			l++;
		}
	}
	
	blip.id = l;
	radarBlips.insert({l, blip});
	
	return l;
}

void GameState::removeBlip(int blip)
{
	auto it = radarBlips.find( blip );
	if ( it != radarBlips.end() )
	{
		radarBlips.erase(it);
	}
}
