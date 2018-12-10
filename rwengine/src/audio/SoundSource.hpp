#ifndef _RWENGINE_SOUND_SOURCE_HPP_
#define _RWENGINE_SOUND_SOURCE_HPP_

#include <rw/filesystem.hpp>

#include <cstdint>

class LoaderSDT;

/// Opaque for raw sound,
/// cooperate with ffmpeg
/// (loading and decoding sound)
class SoundSource {
    friend class SoundManager;
    friend struct SoundBuffer;

public:
    /// Load sound from mp3/wav file
    void loadFromFile(const rwfs::path& filePath);

    /// Load sound from sdt file
    void loadSfx(LoaderSDT& sdt, std::size_t index, bool asWave = true);

private:
    /// Raw data
    std::vector<int16_t> data;

    std::uint32_t channels;
    std::uint32_t sampleRate;
};

#endif
