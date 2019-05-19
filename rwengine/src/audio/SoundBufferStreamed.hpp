#ifndef _RWENGINE_SOUND_BUFFER_STREAMED_HPP_
#define _RWENGINE_SOUND_BUFFER_STREAMED_HPP_

#include "audio/SoundBuffer.hpp"

#include <array>
#include <future>

struct SoundBufferStreamed : public SoundBuffer {
    static constexpr unsigned int kNrBuffersStreaming = 4;
    static constexpr unsigned int kSizeOfChunk = 4096;
    static constexpr std::chrono::milliseconds kTickFreqMs =
        std::chrono::milliseconds(100);

    SoundBufferStreamed();
    ~SoundBufferStreamed() override;
    bool bufferData(SoundSource& soundSource) final;

    void play() final;
    void pause() final;
    void stop() final;

private:
    SoundSource* soundSource = nullptr;
    void updateBuffers();
    unsigned int streamedData = 0;
    unsigned int buffersUsed = 0;
    std::array<ALuint, kNrBuffersStreaming> buffers;
    std::future<void> bufferingThread;
};

#endif
