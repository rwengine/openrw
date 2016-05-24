#include <audio/SoundManager.hpp>
#include <audio/MADStream.hpp>

#include <iostream>

void checkALerror(const std::string& file, unsigned int line);

#if RW_DEBUG
	#define alCheck(stmt) do { stmt; checkALerror(__FILE__, __LINE__); } while(0)
#else
	#define alCheck(stmt) stmt
#endif

SoundManager::SoundManager()
: backgroundNoise(nullptr)
{

}

bool SoundManager::playBackground(const std::string& fileName)
{
	if( backgroundNoise )
	{
		delete backgroundNoise;
	}
	
	sf::Music* bg = new sf::Music;
	
	if( bg->openFromFile( fileName ) )
	{
		backgroundNoise = bg;
		backgroundNoise->setLoop(true);
		bg->play();
		return true;
	}
	
	delete bg;
	return false;
}

void SoundManager::pause(bool p)
{
	if( backgroundNoise )
	{
		if( p )
		{
			backgroundNoise->pause();
		}
		else
		{
			backgroundNoise->play();
		}
	}
}

void checkALerror(const std::string& file, unsigned int line)
{
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		std::cerr << "OpenAL error at " << file << ":" << line << ": ";

		switch (err) {
		case AL_INVALID_NAME:
			std::cerr << "Invalid name!";
			break;
		case AL_INVALID_VALUE:
			std::cerr << "Invalid value!";
			break;
		case AL_INVALID_OPERATION:
			std::cerr << "Invalid operation!";
			break;
		default:
			std::cerr << err;
		}

		std::cerr << std::endl;
	}
}
