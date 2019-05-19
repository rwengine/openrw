#include "audio/SoundBufferStreamed.hpp"

#include <rw/types.hpp>

#include "audio/SoundSource.hpp"
#include "audio/alCheck.hpp"

SoundBufferStreamed::SoundBufferStreamed() {
    alCheck(alGenSources(1, &source));

    alCheck(alGenBuffers(kNrBuffersStreaming, buffers.data()));

    alCheck(alSourcef(source, AL_PITCH, 1));
    alCheck(alSourcef(source, AL_GAIN, 1));
    alCheck(alSource3f(source, AL_POSITION, 0, 0, 0));
    alCheck(alSource3f(source, AL_VELOCITY, 0, 0, 0));
    alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
}

SoundBufferStreamed::~SoundBufferStreamed() {
    std::lock_guard<std::mutex> lock(soundSource->mutex);

    if (state == State::Created) {  // We should only clear queue
        alSourcei(source, AL_BUFFER, 0);
    } else {
        auto nrBuffersToUnqueue = std::min(kNrBuffersStreaming, buffersUsed);
        alCheck(
            alSourceUnqueueBuffers(source, nrBuffersToUnqueue, buffers.data()));
    }

    alCheck(alDeleteBuffers(kNrBuffersStreaming, buffers.data()));
}

bool SoundBufferStreamed::bufferData(SoundSource &soundSource) {
    std::lock_guard<std::mutex> lock(soundSource.mutex);

    /* Rewind the source position and clear the buffer queue */
    alCheck(alSourceRewind(source));
    alCheck(alSourcei(source, AL_BUFFER, 0));

    /* Fill the buffer queue */
    for (auto i = 0u; i < buffers.size() &&
                      streamedData * kSizeOfChunk < soundSource.data.size();
         i++) {
        auto sizeOfNextChunk = std::min(static_cast<size_t>(kSizeOfChunk),
                                        soundSource.data.size());

        alCheck(alBufferData(
            buffers[i],
            soundSource.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            &soundSource.data[streamedData * kSizeOfChunk],
            static_cast<ALsizei>(sizeOfNextChunk * sizeof(int16_t)),
            soundSource.sampleRate));
        streamedData++;
        buffersUsed++;
    }

    alCheck(alSourceQueueBuffers(source, buffersUsed, buffers.data()));

    this->soundSource = &soundSource;

    return true;
}

void SoundBufferStreamed::play() {
    {
        std::lock_guard<std::mutex> lock(soundSource->mutex);
        alSourcePlay(source);

        // Maybe another thread is running, we should tell him to stop
        if (state == State::Playing) {
            state = State::Stopped;
        }
    }
    // Use preloaded data (and give another thread time to stop)
    std::this_thread::sleep_for(kTickFreqMs);

    // Not we should be able start thread
    bufferingThread = std::async(std::launch::async,
                               &SoundBufferStreamed::updateBuffers, this);
}

void SoundBufferStreamed::updateBuffers() {
    state = State::Playing;
    while (true) {
        {
            std::lock_guard<std::mutex> lock(soundSource->mutex);
            if (state == State::Stopped) {
                return;
            }

            ALint processed, state;

            /* Get relevant source info */
            alCheck(alGetSourcei(source, AL_SOURCE_STATE, &state));
            alCheck(alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed));

            bool bufferedData = false;

            /* Unqueue and handle each processed buffer */
            while (processed > 0 &&
                   streamedData * kSizeOfChunk < soundSource->data.size()) {
                bufferedData = true;
                ALuint bufid{};
                auto sizeOfNextChunk = std::min(
                    static_cast<size_t>(kSizeOfChunk),
                    soundSource->data.size() -
                        static_cast<size_t>(kSizeOfChunk) * streamedData);

                alCheck(alSourceUnqueueBuffers(source, 1, &bufid));
                processed--;

                if (sizeOfNextChunk > 0) {
                    alCheck(alBufferData(
                        bufid,
                        soundSource->channels == 1 ? AL_FORMAT_MONO16
                                                   : AL_FORMAT_STEREO16,
                        &soundSource->data[streamedData * kSizeOfChunk],
                        sizeOfNextChunk * sizeof(int16_t),
                        soundSource->sampleRate));
                    streamedData++;
                    buffersUsed++;
                    alCheck(alSourceQueueBuffers(source, 1, &bufid));
                }
            }

            /* Make sure the source hasn't underrun */
            if (bufferedData && state != AL_PLAYING && state != AL_PAUSED) {
                ALint queued;

                /* If no buffers are queued, playback is finished */
                alCheck(alGetSourcei(source, AL_BUFFERS_QUEUED, &queued));
                if (queued == 0) return;

                alCheck(alSourcePlay(source));
            }
        }
        std::this_thread::sleep_for(kTickFreqMs);
    }
}

void SoundBufferStreamed::pause() {
    std::lock_guard<std::mutex> lock(soundSource->mutex);
    state = State::Stopped;
    alCheck(alSourcePause(source));
}
void SoundBufferStreamed::stop() {
    std::lock_guard<std::mutex> lock(soundSource->mutex);
    state = State::Stopped;
    alCheck(alSourceStop(source));
}
