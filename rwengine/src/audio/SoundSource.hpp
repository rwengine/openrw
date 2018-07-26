#ifndef _RWENGINE_SOUND_SOURCE_HPP_
#define _RWENGINE_SOUND_SOURCE_HPP_

#include <rw/filesystem.hpp>
#include <loaders/LoaderSDT.hpp>

/// Opaque for raw sound,
/// cooperate with ffmpeg
/// (loading and decoding sound)
class SoundSource {
    friend class SoundManager;
    friend class SoundBuffer;

public:
    /// Load sound from mp3/wav file
    void loadFromFile(const rwfs::path& filePath);

    /// Load sound from sdt file
    void loadSfx(const rwfs::path& path, LoaderSDT& sdt, size_t index, bool asWave = true);

private:
    /// Raw data
    std::vector<int16_t> data;

    size_t channels;
    size_t sampleRate;
};

#endif
