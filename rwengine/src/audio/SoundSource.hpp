#ifndef _RWENGINE_SOUND_SOURCE_HPP_
#define _RWENGINE_SOUND_SOURCE_HPP_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include <cstdint>
#include <filesystem>
#include <future>
#include <mutex>
#include <vector>

/// Structure for input data
struct InputData {
    uint8_t* ptr = nullptr;
    size_t size{};  ///< size left in the buffer
};

class SwrContext;
class AVFormatContext;
class AVStream;
class AVIOContext;
class LoaderSDT;

/// Opaque for raw sound,
/// cooperate with ffmpeg
/// (loading and decoding sound)
class SoundSource {
    friend class SoundManager;
    friend struct SoundBuffer;
    friend struct SoundBufferStreamed;

public:
    bool allocateAudioFrame();

    bool allocateFormatContext(const std::filesystem::path& filePath);
    bool prepareFormatContextSfx(LoaderSDT& sdt, size_t index, bool asWave);

    bool findAudioStream(const std::filesystem::path& filePath);
    bool findAudioStreamSfx();

    bool prepareCodecContextWrap();
    bool prepareCodecContext();

    bool prepareCodecContextSfxWrap();
    bool prepareCodecContextSfx();

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 5, 0)
    bool prepareCodecContextLegacy();
    bool prepareCodecContextSfxLegacy();
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 37, 100)
    void decodeFramesLegacy(size_t framesToDecode);
#endif

    void decodeFramesWrap(const std::filesystem::path& filePath);
    void decodeFramesSfxWrap();
    void decodeFrames(size_t framesToDecode);
    void decodeAndResampleFrames(const std::filesystem::path& filePath,
                                 size_t framesToDecode);

    void cleanupAfterSoundLoading();
    void cleanupAfterSfxLoading();

    void exposeSoundMetadata();
    void exposeSfxMetadata(LoaderSDT& sdt);

    void decodeRestSoundFramesAndCleanup(const std::filesystem::path& filePath);
    void decodeRestSfxFramesAndCleanup();

    /// Load sound from mp3/wav file
    void loadFromFile(const std::filesystem::path& filePath, bool streaming = false);

    /// Load sound from sdt file
    void loadSfx(LoaderSDT& sdt, std::size_t index, bool asWave = true,
                 bool streaming = false);

    unsigned int decodedFrames = 0u;

private:
    /// Raw data
    std::vector<int16_t> data;

    std::uint32_t channels;
    std::uint32_t sampleRate;

    AVFrame* frame = nullptr;
    AVFormatContext* formatContext = nullptr;
    AVStream* audioStream = nullptr;
    const AVCodec* codec = nullptr;
    SwrContext* swr = nullptr;
    AVCodecContext* codecContext = nullptr;
    AVPacket* readingPacket;

    // For sfx
    AVIOContext* avioContext;
    std::unique_ptr<char[]> raw_sound;
    std::unique_ptr<uint8_t[]> inputDataStart;
    InputData input{};

    std::mutex mutex;
    std::future<void> loadingThread;
};

#endif
