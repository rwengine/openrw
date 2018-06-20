#ifndef _RWENGINE_SOUND_HPP_
#define _RWENGINE_SOUND_HPP_

#include <memory>
#include <vector>
#include <string>
#include <mutex>

#include <al.h>
#include <alc.h>
#include <glm/glm.hpp>

#include "loaders/LoaderSDT.hpp"
#include "rw/defines.hpp"
#include <rw/filesystem.hpp>

/// Opaque for raw sound,
/// cooperate with ffmpeg
/// (loading and decoding sound)
class SoundSource {
    friend class SoundManager;
    friend class SoundBuffer;

public:
    /// Load sound from mp3/wav file
    void loadFromFile(const std::string& filename);

    /// Load sound from sdt file
    void loadSfx(const rwfs::path& path, const size_t& index);

private:
    /// Raw data
    std::vector<int16_t> data;

    size_t channels;
    size_t sampleRate;
};

/// OpenAL tool for playing
/// sound instance.
class SoundBuffer {
    friend class SoundManager;

public:
    SoundBuffer();
    bool bufferData(SoundSource& soundSource);

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void play();
    void pause();
    void stop();

    void setPosition(const glm::vec3 position);
    void setLooping(const bool  looping);
    void setPitch(const float  pitch);
    void setGain(const float  gain);
    void setMaxDistance(const float  maxDist);

private:
    ALuint source;
    ALuint buffer;
};

/// Wrapper for SoundBuffer and SoundSource.
/// Each command connected
/// with playment is passed to SoundBuffer
struct Sound {
    size_t id = 0;
    std::shared_ptr<SoundSource> source = nullptr;
    std::unique_ptr<SoundBuffer> buffer = nullptr;
    bool isLoaded = false;

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

    void play()  {
        buffer->play();
    }

    void pause() {
        buffer->pause();
    }

    void stop() {
        buffer->stop();
    }

    void setPosition(const glm::vec3 position) {
        buffer->setPosition(position);
    }

    void setLooping(const bool  looping) {
        buffer->setLooping(looping);
    }

    void setPitch(const float  pitch) {
        buffer->setPitch(pitch);
    }

    void setGain(const float  gain) {
        buffer->setGain(gain);
    }

    void setMaxDistance(const float  maxDist) {
        buffer->setMaxDistance(maxDist);
    }

    int getScriptObjectID() const {
        return id;
    }
};
#endif
