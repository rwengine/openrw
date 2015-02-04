#include <audio/SoundManager.hpp>
#include <audio/MADStream.hpp>

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
	if( p )
	{
		backgroundNoise->pause();
	}
	else
	{
		backgroundNoise->play();
	}
}
