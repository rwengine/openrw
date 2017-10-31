#ifndef _RWENGINE_SOUNDMANAGER_HPP_
#define _RWENGINE_SOUNDMANAGER_HPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <al.h>
#include <alc.h>

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    bool loadSound(const std::string& name, const std::string& fileName);
    bool isLoaded(const std::string& name);
    void playSound(const std::string& name);
    void pauseSound(const std::string& name);
    bool isPlaying(const std::string& name);

    bool playBackground(const std::string& fileName);

    bool loadMusic(const std::string& name, const std::string& fileName);
    void playMusic(const std::string& name);
    void stopMusic(const std::string& name);

    void pause(bool p);

private:
    class SoundSource {
        friend class SoundManager;
        friend class SoundBuffer;

    public:
        void loadFromFile(const std::string& filename);

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

#endif
