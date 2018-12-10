#include "audio/SoundBuffer.hpp"

#include <rw/types.hpp>

#include "audio/alCheck.hpp"
#include "audio/SoundSource.hpp"

SoundBuffer::SoundBuffer() {
    alCheck(alGenSources(1, &source));
    alCheck(alGenBuffers(1, &buffer));

    alCheck(alSourcef(source, AL_PITCH, 1));
    alCheck(alSourcef(source, AL_GAIN, 1));
    alCheck(alSource3f(source, AL_POSITION, 0, 0, 0));
    alCheck(alSource3f(source, AL_VELOCITY, 0, 0, 0));
    alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
}

SoundBuffer::~SoundBuffer() {
    alCheck(alDeleteSources(1, &source));
    alCheck(alDeleteBuffers(1, &buffer));
}

bool SoundBuffer::bufferData(SoundSource& soundSource) {
    alCheck(alBufferData(
        buffer,
        soundSource.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        &soundSource.data.front(),
        static_cast<ALsizei>(soundSource.data.size() * sizeof(int16_t)),
        soundSource.sampleRate));
    alCheck(alSourcei(source, AL_BUFFER, buffer));

    return true;
}

bool SoundBuffer::isPlaying() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE, &sourceState));
    return AL_PLAYING == sourceState;
}

bool SoundBuffer::isPaused() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE, &sourceState));
    return AL_PAUSED == sourceState;
}

bool SoundBuffer::isStopped() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE, &sourceState));
    return AL_STOPPED == sourceState;
}

void SoundBuffer::play() {
    alCheck(alSourcePlay(source));
}
void SoundBuffer::pause() {
    alCheck(alSourcePause(source));
}
void SoundBuffer::stop() {
    alCheck(alSourceStop(source));
}

void SoundBuffer::setPosition(const glm::vec3& position) {
    alCheck(
        alSource3f(source, AL_POSITION, position.x, position.y, position.z));
}

void SoundBuffer::setLooping(bool looping) {
    if (looping) {
        alCheck(alSourcei(source, AL_LOOPING, AL_TRUE));
    } else {
        alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
    }
}

void SoundBuffer::setPitch(float pitch) {
    alCheck(alSourcef(source, AL_PITCH, pitch));
}
void SoundBuffer::setGain(float gain) {
    alCheck(alSourcef(source, AL_GAIN, gain));
}
void SoundBuffer::setMaxDistance(float maxDist) {
    alCheck(alSourcef(source, AL_MAX_DISTANCE, maxDist));
}
