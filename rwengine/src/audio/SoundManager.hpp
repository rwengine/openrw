#pragma once

#include "al.h"
#include "alc.h"

#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <loaders/LoaderSDT.hpp>

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    bool loadSound(const std::string& name, const std::string& fileName, const size_t index = 0);
    bool isLoaded(const std::string& name);
    void playSound(const std::string& name);
    void playSound(const std::string& name, const glm::vec3 position, const bool looping = false, const float maxDist = 30.f); 
    void pauseSound(const std::string& name);
    bool isPlaying(const std::string& name);

    bool playBackground(const std::string& fileName);

    bool loadMusic(const std::string& name, const std::string& fileName);
    void playMusic(const std::string& name);
    void stopMusic(const std::string& name);

    void setListenerPosition(const glm::vec3 position);
    void setListenerVelocity(const glm::vec3 vel);
    void setListenerOrientation(const glm::vec3 at, const glm::vec3 up  = glm::vec3(0.f, 0.f, 1.f));

    void setSoundPosition(const std::string name,const glm::vec3 position);

    void pause(bool p);

private:
    class SoundSource {
        friend class SoundManager;
        friend class SoundBuffer;

    public:
        void loadFromFile(const std::string& filename, const size_t index = 0);

    private:
        std::vector<int16_t> data;

        size_t channels;
        size_t sampleRate;
    };

    class SoundBuffer {
        friend class SoundManager;

    public:
        SoundBuffer();
        bool bufferData(SoundSource& soundSource);

    private:
        ALuint source;
        ALuint buffer;
    };

    struct Sound {
        SoundSource source;
        SoundBuffer buffer;
        bool isLoaded = false;
    };

    bool initializeOpenAL();
    bool initializeAVCodec();

    ALCcontext* alContext = nullptr;
    ALCdevice* alDevice = nullptr;

    std::map<std::string, Sound> sounds;
    std::string backgroundNoise;
};
