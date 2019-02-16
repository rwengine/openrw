#ifndef _RWENGINE_SOUND_HPP_
#define _RWENGINE_SOUND_HPP_

#include <glm/vec3.hpp>

#include <memory>

class SoundSource;
class EffectSlot;
struct SoundBuffer;

/// Wrapper for SoundBuffer and SoundSource.
/// Each command connected
/// with playment is passed to SoundBuffer
struct Sound {
    size_t id = 0;
    bool isLoaded = false;

    std::shared_ptr<SoundSource> source;
    std::unique_ptr<SoundBuffer> buffer;

    Sound() = default;
    ~Sound() = default;

    bool isPlaying() const;

    bool isPaused() const;
    bool isStopped() const;

    void play();

    void pause();

    void stop();

    void setPosition(const glm::vec3& position);

    void setLooping(bool looping);

    void setPitch(float pitch);

    void setGain(float gain);

    void setMaxDistance(float maxDist);

    void attachToEffectSlot(const std::shared_ptr<EffectSlot> effectSlot);

    size_t getScriptObjectID() const;
};
#endif
