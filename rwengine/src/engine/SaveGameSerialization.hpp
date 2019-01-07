#ifndef _RWENGINE_SAVEGAMESERIALIZATION_HPP_
#define _RWENGINE_SAVEGAMESERIALIZATION_HPP_
#include <rw/debug.hpp>

#include <iostream>

namespace SaveGameSerialization {
class Reader {
    std::istream& _stream;
public:
    static constexpr bool Writing = false;

    explicit Reader(std::istream& stream)
        : _stream(stream)
    {}

    template <class R>
    bool serialize(R& out) {
        _stream.read(reinterpret_cast<char*>(&out), sizeof(R));
        return _stream.gcount() == sizeof(R);
    }

    void seek(std::streamoff diff) {
        _stream.seekg(diff, std::ios_base::cur);
    }

    auto f() { return _stream.tellg(); }
};

template <class Tserializer>
class SaveBlock {
    Tserializer& _inner;
    uint32_t _blockSize {};
    uint32_t _cursor {};
public:
    static constexpr bool Writing = Tserializer::Writing;

    SaveBlock(const SaveBlock&) = delete;

    explicit SaveBlock(Tserializer& inner)
        : _inner(inner) {
        if constexpr (!Writing) {
            auto result = inner.serialize(_blockSize);
            RW_UNUSED(result);
            RW_ASSERT(result);
        }
    }

    uint32_t size() const {
        return _blockSize;
    }

    template <class R>
    bool serialize(R& field) {
        if constexpr (Writing) _blockSize += sizeof(field);
        else _cursor += sizeof(field);
        RW_ASSERT(_cursor <= _blockSize);
        return _inner.serialize(field);
    }

    void seek(std::streamoff diff) {
        if constexpr (!Writing) {
            _cursor += diff;
            _inner.seek(diff);
        }
    }

    auto f() { return _inner.f(); }

    ~SaveBlock() {
        if constexpr (Writing) {
            // @TODO
        }
        else if (_cursor < _blockSize) {
            _inner.seek(_blockSize - _cursor);
        }
    }
};

template<class Tserializer>
SaveBlock<Tserializer> MakeSaveBlock(Tserializer& inner) {
    return SaveBlock<Tserializer> {inner};
}

template<class Tserializer, class Tfunc>
auto WithBlock(Tserializer& s, const Tfunc& l) {
    auto b = MakeSaveBlock(s);
    return l(b);
}

} // namespace SaveGameSerialization

#include <engine/SaveGameSerializers.inl>

#endif //_RWENGINE_SAVEGAMESERIALIZATION_HPP_
