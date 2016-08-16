#include <engine/GameState.hpp>

BasicState::BasicState()
	: saveName { 0 }
	, saveTime { 0, 0, 0, 0, 0, 0, 0, 0 }
	, islandNumber { 0 }
	, cameraPosition { }
	, gameMinuteMS { 0 }
	, lastTick { 0 }
	, gameHour { 0 }
	, gameMinute { 0 }
	, padMode { 0 }
	, timeMS { 0 }
	, timeScale { 0 }
	, timeStep { 0 }
	, timeStep_unclipped { 0 }
	, frameCounter { 0 }
	, timeStep2 { 0 }
	, framesPerUpdate { 0 }
	, timeScale2 { 0 }
	, lastWeather { 0 }
	, nextWeather { 0 }
	, forcedWeather { 0 }
	, weatherInterpolation { 0 }
	, weatherType { 0 }
	, cameraData { 0 }
	, cameraData2 { 0 }
{ }

PlayerInfo::PlayerInfo()
	: money { 0 }
	, displayedMoney { 0 }
	, hiddenPackagesCollected { 0 }
	, hiddenPackageCount { 0 }
	, neverTired { 0 }
	, fastReload { 0 }
	, thaneOfLibertyCity { 0 }
	, singlePayerHealthcare { 0 }
{ }

GameStats::GameStats()
	: playerKills { 0 }
	, otherKills { 0 }
	, carsExploded { 0 }
	, shotsHit { 0 }
	, pedTypesKilled { }
	, helicoptersDestroyed { 0 }
	, playerProgress { 0 }
	, explosiveKgsUsed { 0 }
	, bulletsFired { 0 }
	, bulletsHit { 0 }
	, carsCrushed { 0 }
	, headshots { 0 }
	, timesBusted { 0 }
	, timesHospital { 0 }
	, daysPassed { 0 }
	, mmRainfall { 0 }
	, insaneJumpMaxDistance { 0 }
	, insaneJumpMaxHeight { 0 }
	, insaneJumpMaxFlips { 0 }
	, insangeJumpMaxRotation { 0 }
	, bestStunt { 0 }
	, uniqueStuntsFound { 0 }
	, uniqueStuntsTotal { 0 }
	, missionAttempts { 0 }
	, missionsPassed { 0 }
	, passengersDroppedOff { 0 }
	, taxiRevenue { 0 }
	, portlandPassed { 0 }
	, stauntonPassed { 0 }
	, shoresidePassed { 0 }
	, bestTurismoTime { 0 }
	, distanceWalked { 0 }
	, distanceDriven { 0 }
	, patriotPlaygroundTime { 0 }
	, aRideInTheParkTime { 0 }
	, grippedTime { 0 }
	, multistoryMayhemTime { 0 }
	, peopleSaved { 0 }
	, criminalsKilled { 0 }
	, highestParamedicLevel { 0 }
	, firesExtinguished { 0 }
	, longestDodoFlight { 0 }
	, bombDefusalTime { 0 }
	, rampagesPassed { 0 }
	, totalRampages { 0 }
	, totalMissions { 0 }
	, highestScore { }
	, peopleKilledSinceCheckpoint { 0 }
	, peopleKilledSinceLastBustedOrWasted { 0 }
	, lastMissionGXT { "" }
{ }

GameState::GameState()
	: basic{}
	, gameTime(0.f)
	, currentProgress(0)
	, maxProgress(1)
	, maxWantedLevel(0)
	, playerObject(0)
	, scriptOnMissionFlag(nullptr)
	, fadeOut(true)
	, fadeStart(0.f)
	, fadeTime(0.f)
	, fadeSound(false)
	, skipCutscene(false)
	, isIntroPlaying(false)
	, currentCutscene(nullptr)
	, cutsceneStartTime(-1.f)
	, isCinematic(false)
	, cameraNear(0.1f)
	, cameraFixed(false)
	, cameraTarget(0)
	, importExportPortland(0)
	, importExportShoreside(0)
	, importExportUnused(0)
	, world(nullptr)
	, script(nullptr)
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
