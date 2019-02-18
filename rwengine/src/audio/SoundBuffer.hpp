#ifndef _RWENGINE_SOUND_BUFFER_HPP_
#define _RWENGINE_SOUND_BUFFER_HPP_

#include <al.h>
#include <glm/vec3.hpp>

#include <memory>

class EffectSlot;
class SoundSource;

/// OpenAL tool for playing
/// sound instance.
struct SoundBuffer {
    SoundBuffer();
    ~SoundBuffer();
    bool bufferData(SoundSource& soundSource);

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

    void attachToEffectSlot(const std::shared_ptr<EffectSlot> &effectSlot);
    void detachFromEffectSlot(const std::shared_ptr<EffectSlot> &effectSlot);

    ALuint source;
    ALuint buffer;
};

#endif
