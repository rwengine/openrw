#ifndef _RWENGINE_SOUND_HPP_
#define _RWENGINE_SOUND_HPP_

#include <memory>
#include <string>
#include <vector>

#include <al.h>
#include <alc.h>
#include <glm/glm.hpp>

#include <rw/filesystem.hpp>
#include <rw/types.hpp>

#include "audio/SoundBuffer.hpp"
#include "audio/SoundSource.hpp"

/// Wrapper for SoundBuffer and SoundSource.
/// Each command connected
/// with playment is passed to SoundBuffer
struct Sound {
    size_t id = 0;
    bool isLoaded = false;

    std::shared_ptr<SoundSource> source = nullptr;
    std::unique_ptr<SoundBuffer> buffer = nullptr;

    Sound() = default;

    bool isPlaying() const {
        return buffer->isPlaying();
    }

    bool isPaused() const {
        return buffer->isPaused();
    }
    bool isStopped() const {
        return buffer->isStopped();
    }

    void play() {
        buffer->play();
    }

    void pause() {
        buffer->pause();
    }

    void stop() {
        buffer->stop();
    }

    void setPosition(const glm::vec3& position) {
        buffer->setPosition(position);
    }

    void setLooping(bool looping) {
        buffer->setLooping(looping);
    }

    void setPitch(float pitch) {
        buffer->setPitch(pitch);
    }

    void setGain(float gain) {
        buffer->setGain(gain);
    }

    void setMaxDistance(float maxDist) {
        buffer->setMaxDistance(maxDist);
    }

    int getScriptObjectID() const {
        return id;
    }
};
#endif
