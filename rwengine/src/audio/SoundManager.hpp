#ifndef _RWENGINE_SOUNDMANAGER_HPP_
#define _RWENGINE_SOUNDMANAGER_HPP_

#include "audio/Sound.hpp"
#include "audio/SoundEffect.hpp"

#include <alc.h>

#include <glm/vec3.hpp>

#include <rw/filesystem.hpp>
#include <loaders/LoaderSDT.hpp>

#include <string>
#include <unordered_map>

class GameWorld;
class ViewCamera;

/// Game's sound manager.
/// It handles all stuff connected with sounds.
/// Worth noted: there are three types of sounds,
/// these containg raw source and openAL buffer for playing (only one instance
/// simultaneously), these containg only source or buffer. (It allows multiple
/// instances simultaneously without duplicating raw source).
class SoundManager {
public:
    SoundManager();
    SoundManager(GameWorld* engine);
    ~SoundManager();

    /// Load sound from file and store it with selected name
    bool loadSound(const std::string& name, const std::string& fileName);

    /// Load selected sfx sound
    void loadSound(size_t index);

    Sound& getSfxBufferRef(size_t name);
    Sound& getSfxSourceRef(size_t name);
    Sound& getSoundRef(const std::string& name);

    size_t createSfxInstance(size_t index);

    /// Checking is selected sound loaded.
    bool isLoaded(const std::string& name);

    /// Checking is selected sound playing.
    bool isPlaying(const std::string& name);

    /// Checking is selected sound playing.
    bool isStopped(const std::string& name);

    /// Checking is selected sound playing.
    bool isPaused(const std::string& name);

    /// Play sound with selected name
    void playSound(const std::string& name);

    /// Erase sound with selected name
    void eraseSound(const std::string& name);

    /// Effect same as playSound with one parametr,
    /// but this function works for sfx and
    /// allows also for setting position,
    /// looping and max Distance.
    /// -1 means no limit of max distance.
    void playSfx(size_t name, const glm::vec3& position, bool looping = false, int maxDist = -1);

    void playSfx(size_t name, const glm::vec3& position, SoundEffect::Type effect = SoundEffect::Type::Reverb,
                 bool looping = false, int maxDist = -1);

    void pauseAllSounds();
    void resumeAllSounds();

    /// Play background from selected file.
    bool playBackground(const std::string& fileName);

    bool loadMusic(const std::string& name, const std::string& fileName);
    void playMusic(const std::string& name);
    void stopMusic(const std::string& name);

    /// Updating listener tranform, called by main loop of game.
    void updateListenerTransform(const ViewCamera& cam);

    /// Setting position of sound source in buffer.
    void setSoundPosition(const std::string& name, const glm::vec3& position);

    void pause(bool p);

    void setVolume(float vol);
    float getCalculatedVolumeOfEffects() const;
    float getCalculatedVolumeOfMusic() const;

private:
    bool initializeOpenAL();
    void initializeAVCodec();

    /// Initialize EFX OpenAL extention
    bool initializeEFX();

    void deinitializeOpenAL();

    ALCcontext* alContext = nullptr;
    ALCdevice* alDevice = nullptr;

    /// Containers for sounds
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<size_t, Sound> sfx;
    std::unordered_map<size_t, Sound> buffers;

    std::string backgroundNoise;

    /// Nr of already created buffers
    size_t bufferNr = 0;

    GameWorld* _engine;
    LoaderSDT sdt{};

    /// Sound volume
    float _volume = 1.f;

    /// Multiplier for effects
    float _effectsVolume = 0.2f;

    /// Multiplier for music and cutscenes audio
    float _musicVolume = 1.f;

    /// Available sound effects for sfx
    std::vector<std::shared_ptr<SoundEffect>> soundEffects;
};

#endif
