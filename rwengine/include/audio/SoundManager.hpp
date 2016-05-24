#pragma once

#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <vector>
#include <SFML/Audio.hpp>

class SoundManager
{
public:
	SoundManager();
	
	void playSound(const std::string& fileName);
	
	bool playBackground(const std::string& fileName);
	
	void pause(bool p);
	
private:

	struct PlayingSound
	{
		sf::Sound sound;
		sf::SoundBuffer buffer;
	};

	std::vector<PlayingSound> sounds;
	
	sf::SoundStream* backgroundNoise;

};