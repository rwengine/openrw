#ifndef _RWENGINE_SOUND_BUFFER_HPP_
#define _RWENGINE_SOUND_BUFFER_HPP_

#include <al.h>
#include <alc.h>
#include <glm/glm.hpp>

#include "audio/SoundSource.hpp"

/// OpenAL tool for playing
/// sound instance.
struct SoundBuffer {
    SoundBuffer();
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

    ALuint source;
    ALuint buffer;
};

#endif
