#pragma once

#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <map>
#include <string>
#include <vector>
#include <SFML/Audio.hpp>

class SoundManager
{
public:
	SoundManager();
	
	bool loadSound(const std::string& name, const std::string& fileName);
	void playSound(const std::string& fileName);
	
	bool playBackground(const std::string& fileName);
	
	void pause(bool p);
	
private:

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

	struct Sound
	{
		SoundSource source;
		SoundBuffer buffer;
		bool isLoaded = false;
	};

	bool initializeOpenAL();

	ALCcontext* alContext;
	ALCdevice* alDevice;

	std::map<std::string, Sound> sounds;
	
	sf::SoundStream* backgroundNoise;

};
