#ifndef _RWENGINE_SOUND_BUFFER_HPP_
#define _RWENGINE_SOUND_BUFFER_HPP_

#include <al.h>
#include <glm/vec3.hpp>

class SoundSource;

/// OpenAL tool for playing
/// sound instance.
struct SoundBuffer {
    SoundBuffer();
    virtual ~SoundBuffer();
    virtual bool bufferData(SoundSource& soundSource);

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    virtual void play();
    virtual void pause();
    virtual void stop();

    void setPosition(const glm::vec3& position);
    void setLooping(bool looping);
    void setPitch(float pitch);
    void setGain(float gain);
    void setMaxDistance(float maxDist);

    ALuint source;
    bool running = false;
private:
    ALuint buffer;
};

#endif
