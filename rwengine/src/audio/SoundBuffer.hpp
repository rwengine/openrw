#ifndef _RWENGINE_SOUND_BUFFER_HPP_
#define _RWENGINE_SOUND_BUFFER_HPP_

#include <al.h>
#include <glm/vec3.hpp>

#include <memory>

class SoundEffect;
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

    void enableEffect(std::shared_ptr<SoundEffect> effect);
    void disableEffect(std::shared_ptr<SoundEffect> effect);

    ALuint source;
    ALuint buffer;
};

#endif
