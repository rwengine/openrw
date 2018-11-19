#include "loaders/LoaderGXT.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <fonts/GameTexts.hpp>
#include <platform/FileHandle.hpp>

void LoaderGXT::load(GameTexts &texts, const FileContentsInfo &file) {
    auto data = file.data.get();

    data += 4;  // TKEY

    std::uint32_t blocksize = *reinterpret_cast<std::uint32_t *>(data);

    data += 4;

    auto tdata = data + blocksize + 8;

    for (size_t t = 0; t < blocksize / 12; ++t) {
        size_t offset = *reinterpret_cast<std::uint32_t *>(data + (t * 12 + 0));
        std::string id(data + (t * 12 + 4));

        auto stringSrc = reinterpret_cast<GameStringChar *>(tdata + offset);
        GameString string(stringSrc);
        texts.addText(id, std::move(string));
    }
}
