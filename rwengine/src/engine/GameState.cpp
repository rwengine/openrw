#include <engine/GameState.hpp>

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
