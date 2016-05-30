#include <audio/SoundManager.hpp>
#include <audio/MADStream.hpp>

#include "audio/alCheck.hpp"
#include "audio/MADStream.hpp"

#include <array>
#include <iostream>

void SoundManager::SoundSource::loadFromFile(const std::string& filename)
{
	fileInfo.format = 0;
	file = sf_open(filename.c_str(), SFM_READ, &fileInfo);

	if (file) {
		size_t numRead = 0;
		std::array<int16_t, 4096> readBuffer;

		while ((numRead = sf_read_short(file, readBuffer.data(), readBuffer.size())) != 0) {
			data.insert(data.end(), readBuffer.begin(), readBuffer.begin() + numRead);
		}
	} else {
		std::cerr << "Error opening sound file \"" << filename << "\": " << sf_strerror(file) << std::endl;
	}
}

SoundManager::SoundBuffer::SoundBuffer()
{
	alCheck(alGenSources(1, &source));
	alCheck(alGenBuffers(1, &buffer));

	alCheck(alSourcef(source, AL_PITCH, 1));
	alCheck(alSourcef(source, AL_GAIN, 1));
	alCheck(alSource3f(source, AL_POSITION, 0, 0, 0));
	alCheck(alSource3f(source, AL_VELOCITY, 0, 0, 0));
	alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
}

bool SoundManager::SoundBuffer::bufferData(SoundSource& soundSource)
{
	alCheck(alBufferData(
		buffer,
		soundSource.fileInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		&soundSource.data.front(),
		soundSource.data.size() * sizeof(uint16_t),
		soundSource.fileInfo.samplerate
	));
	alCheck(alSourcei(source, AL_BUFFER, buffer));

	return true;
}

SoundManager::SoundManager()
{
	initializeOpenAL();
}

bool SoundManager::initializeOpenAL()
{
	alDevice = alcOpenDevice(NULL);
	if ( ! alDevice) {
		std::cerr << "Could not find OpenAL device!" << std::endl;
		return false;
	}

	alContext = alcCreateContext(alDevice, NULL);
	if ( ! alContext) {
		std::cerr << "Could not create OpenAL context!" << std::endl;
		return false;
	}

	if ( ! alcMakeContextCurrent(alContext)) {
		std::cerr << "Unable to make OpenAL context current!" << std::endl;
		return false;
	}

	return true;
}

bool SoundManager::loadSound(const std::string& name, const std::string& fileName)
{
	Sound* sound = nullptr;
	auto sound_iter = sounds.find(name);

	if (sound_iter != sounds.end()) {
		sound = &sound_iter->second;
	} else {
		auto emplaced = sounds.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple());
		sound = &emplaced.first->second;

		sound->source.loadFromFile(fileName);
		sound->isLoaded = sound->buffer.bufferData(sound->source);
	}

	return sound->isLoaded;
}
bool SoundManager::isLoaded(const std::string& name)
{
	if (sounds.find(name) != sounds.end()) {
		return sounds[name].isLoaded;
	}

	return false;
}
void SoundManager::playSound(const std::string& name)
{
	if (sounds.find(name) != sounds.end()) {
		alCheck(alSourcePlay(sounds[name].buffer.source));
	}
}
void SoundManager::pauseSound(const std::string& name)
{
	if (sounds.find(name) != sounds.end()) {
		alCheck(alSourcePause(sounds[name].buffer.source));
	}
}
bool SoundManager::isPlaying(const std::string& name)
{
	if (sounds.find(name) != sounds.end()) {
		ALint sourceState;
		alCheck(alGetSourcei(sounds[name].buffer.source, AL_SOURCE_STATE, &sourceState));
		return AL_PLAYING == sourceState;
	}

	return false;
}

bool SoundManager::playBackground(const std::string& fileName)
{
	if (this->loadSound(fileName, fileName)) {
		backgroundNoise = fileName;
		this->playSound(fileName);
		return true;
	}

	return false;
}

bool SoundManager::loadMusic(const std::string& name, const std::string& fileName)
{
	MADStream* music = nullptr;
	auto music_iter = musics.find(name);

	if (music_iter != musics.end()) {
		music = &music_iter->second;
	} else {
		auto emplaced = musics.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple());
		music = &emplaced.first->second;
	}

	return music->openFromFile(fileName);
}
void SoundManager::playMusic(const std::string& name)
{
	auto music = musics.find(name);
	if (music != musics.end()) {
		music->second.play();
	}
}
void SoundManager::stopMusic(const std::string& name)
{
	auto music = musics.find(name);
	if (music != musics.end()) {
		music->second.stop();
	}
}

void SoundManager::pause(bool p)
{
	if (backgroundNoise.length() > 0) {
		if (p) {
			pauseSound(backgroundNoise);
		} else {
			playSound(backgroundNoise);
		}
	}
}
