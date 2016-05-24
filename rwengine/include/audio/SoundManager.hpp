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

	class SoundSource
	{
		friend class SoundManager;
		friend class SoundBuffer;
	public:
		void loadFromFile(const std::string& filename);
	private:
		SF_INFO fileInfo;
		SNDFILE* file;
		std::vector<uint16_t> data;
	};

	class SoundBuffer
	{
		friend class SoundManager;
	public:
		SoundBuffer();
		bool bufferData(SoundSource& soundSource);
	private:
		ALuint source;
		ALuint buffer;
	};

	bool initializeOpenAL();

	ALCcontext* alContext;
	ALCdevice* alDevice;

	std::vector<PlayingSound> sounds;
	
	sf::SoundStream* backgroundNoise;

};