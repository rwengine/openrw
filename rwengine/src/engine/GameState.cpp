#include <engine/GameState.hpp>

GameState::GameState()
	: basic{}
	, playerInfo{}
	, gameStats{}
	, gameTime(0.f),
currentProgress(0),
maxProgress(1),
maxWantedLevel(0),
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
cameraNear(0.1f),
cameraFixed(false),
cameraTarget(0),
world(nullptr),
script(nullptr)
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
