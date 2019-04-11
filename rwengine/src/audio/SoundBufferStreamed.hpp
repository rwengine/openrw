#ifndef _RWENGINE_SOUND_BUFFER_STREAMED_HPP_
#define _RWENGINE_SOUND_BUFFER_STREAMED_HPP_

#include "audio/SoundBuffer.hpp"

#include <array>
#include <future>

struct SoundBufferStreamed : public SoundBuffer {
    static constexpr unsigned int kNrBuffersStreaming = 4;
    static constexpr unsigned int kSizeOfChunk = 4096;

    SoundBufferStreamed();
    ~SoundBufferStreamed() override;
    bool bufferData(SoundSource& soundSource) final;

    virtual void play() final;

private:
    SoundSource* soundSource = nullptr;
    void updateBuffers();
    unsigned int streamedData = 0;
    std::array<ALuint, kNrBuffersStreaming> buffers;
    std::future<void> loadingThread;
};

#endif
