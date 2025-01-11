#ifndef _LIBRW_RWBINARYSTREAM_HPP_
#define _LIBRW_RWBINARYSTREAM_HPP_

#include <glm/gtc/type_precision.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

#include "rw/debug.hpp"
#include "rw/casts.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

/**
 * @brief Class for working with RenderWare binary streams.
 *
 * Stream files are split into chunks, each of which may have numerous
 * child chunks (in particular, the "struct" chunk which is used to store
 * data relating to the parent chunk).
 */
class RWBStream {
    char* _data;
    std::ptrdiff_t _size;
    char* _dataCur;
    char* _nextChunk;
    std::uint32_t _chunkStamp;
    size_t _currChunkSz;

public:
    typedef std::uint32_t ChunkID;

    RWBStream(char* data, size_t size)
        : _data(data), _size(size), _dataCur(data), _nextChunk(data) {
    }

    /**
     * Moves the stream to the next chunk and returns it's ID
     */
    ChunkID getNextChunk() {
        // Check that there's any data left
        if ((_dataCur - _data) >= _size) return 0;

        // _nextChunk is initally = to _data, making this a non-op
        _dataCur = _nextChunk;

        ChunkID id = bit_cast<std::uint32_t>(*_dataCur);
        _dataCur += sizeof(ChunkID);

        _currChunkSz = bit_cast<std::uint32_t>(*_dataCur);
        _dataCur += sizeof(std::uint32_t);

        _chunkStamp = bit_cast<std::uint32_t>(*_dataCur);
        _dataCur += sizeof(std::uint32_t);

        _nextChunk = _dataCur + _currChunkSz;

        return id;
    }

    char* getCursor() const {
        return _dataCur;
    }

    size_t getCurrentChunkSize() const {
        return _currChunkSz;
    }

    std::uint32_t getChunkVersion() const {
        if (_chunkStamp & 0xFFFF0000)
	    return ((_chunkStamp >> 14 & 0x3FF00) + 0x30000) |
		    (_chunkStamp >> 16 & 0x3F);
	return _chunkStamp << 8;
    }

    /**
     * @brief Returns a new stream for the data inside this one.
     */
    RWBStream getInnerStream() const {
        return {_dataCur, _currChunkSz};
    }
};

/**
 * @file rwbinarystream.h
 * Deprecated RenderWare binary stream interface.
 * Contains the structs for the shared Render Ware binary stream data.
 * Many thanks to
 * http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file
 */

namespace RW {
enum {
    SID_Struct = 0x0001,
    SID_String = 0x0002,
    SID_Extension = 0x0003,

    SID_Texture = 0x0006,
    SID_Material = 0x0007,
    SID_MaterialList = 0x0008,

    SID_FrameList = 0x000E,
    SID_Geometry = 0x000F,
    SID_Clump = 0x0010,

    SID_Atomic = 0x0014,
    SID_TextureNative = 0x0015,
    SID_TextureDictionary = 0x0016,

    SID_GeometryList = 0x001A,

    SID_HAnimPLG = 0x011E,

    SID_BinMeshPLG = 0x50E,

    SID_NodeName = 0x0253F2FE
};

typedef glm::vec3 BSTVector3;

typedef glm::mat3 BSTMatrix;

struct BSSectionHeader {
    uint32_t id;
    uint32_t size;
    uint32_t versionid;
};

struct BSExtension {};

struct BSFrameList {
    uint32_t numframes;
};

struct BSClump {
    uint32_t numatomics;
};

struct BSStruct {
    uint32_t id;  // = 0x0001
};

struct BSGeometryList {
    uint32_t numgeometry;
};

struct BSGeometry {
    uint16_t flags;
    uint8_t numuvs;
    uint8_t geomflags;
    uint32_t numtris;
    uint32_t numverts;
    uint32_t numframes;

    enum {
        IsTriangleStrip = 0x1,
        VertexTranslation = 0x2,
        TexCoords1 = 0x4,
        VertexColors = 0x8,
        StoreNormals = 0x16,
        DynamicVertexLighting = 0x32,
        ModuleMaterialColor = 0x64,
        TexCoords2 = 0x128
    };
};

typedef glm::u8vec4 BSColor;

struct BSGeometryColor {
    BSColor ambient;
    BSColor diffuse;
    BSColor specular;
};

struct BSGeometryUV {
    float u;
    float v;
};

struct BSGeometryTriangle {
    uint16_t first;
    uint16_t second;
    uint16_t attrib;  // Who designed this nonsense.
    uint16_t third;
};

struct BSGeometryBounds {
    BSTVector3 center;
    float radius;
    uint32_t positions;
    uint32_t normals;
};

struct BSMaterialList {
    uint32_t nummaterials;
};

struct BSMaterial {
    uint32_t unknown;
    BSColor color;
    uint32_t alsounknown;
    uint32_t numtextures;
    float ambient;
    float specular;
    float diffuse;
};

struct BSTexture {
    uint16_t filterflags;
    uint16_t unknown;
};

struct BSBinMeshPLG {
    uint32_t facetype;
    uint32_t numsplits;
    uint32_t numfaces;
};

struct BSMaterialSplit {
    uint32_t numverts;
    uint32_t index;
};

/**
 * Texture Dictionary Structures (TXD)
 */
struct BSTextureDictionary {
    uint16_t numtextures;
    uint16_t unknown;
};

struct BSTextureNative {
    uint32_t platform;
    uint16_t filterflags;
    uint8_t wrapV;
    uint8_t wrapU;
    char diffuseName[32];
    char alphaName[32];
    uint32_t rasterformat;
    uint32_t alpha;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t nummipmaps;
    uint8_t rastertype;
    uint8_t dxttype;
    uint32_t datasize;

    enum {
        FILTER_NONE = 0x0,
        FILTER_NEAREST = 0x01,
        FILTER_LINEAR = 0x02,
        FILTER_MIP_NEAREST = 0x03,
        FILTER_MIP_LINEAR = 0x04,
        FILTER_LINEAR_MIP_NEAREST = 0x05,
        FILTER_LINEAR_MIP_LINEAR = 0x06,
        FILTER_MYSTERY_OPTION = 0x1101
    };

    enum {
        WRAP_NONE = 0x00,
        WRAP_WRAP = 0x01,
        WRAP_MIRROR = 0x02,
        WRAP_CLAMP = 0x03
    };

    enum {
        FORMAT_DEFAULT = 0x0000,  // helpful
        FORMAT_1555 = 0x0100,     // Alpha 1, RGB 5 b
        FORMAT_565 = 0x0200,      // 5r6g5b
        FORMAT_4444 = 0x0300,     // 4 bits each
        FORMAT_LUM8 = 0x0400,     // Greyscale
        FORMAT_8888 = 0x0500,     // 8 bits each
        FORMAT_888 = 0x0600,      // RGB 8 bits each
        FORMAT_555 = 0x0A00,      // do not use

        FORMAT_EXT_AUTO_MIPMAP = 0x1000,  // Generate mipmaps
        FORMAT_EXT_PAL8 = 0x2000,         // 256 colour palette
        FORMAT_EXT_PAL4 = 0x4000,         // 16 color palette
        FORMAT_EXT_MIPMAP = 0x8000        // Mipmaps included
    };
};

struct BSPaletteData {
    uint32_t palette[256];
    uint32_t rastersize;
};

/**
 * Structure object
 */
class BinaryStreamSection {
public:
    /**
     * Data pointer
     */
    char* data;

    /**
     * Offset of this section in the data
     */
    size_t offset;

    /**
     * The BSSectionHeader for the section
     */
    BSSectionHeader header;

    /**
     * Structure header
     */
    BSSectionHeader* structure;

    BinaryStreamSection(char* data, size_t offset = 0)
        : data(data), offset(offset), structure(nullptr) {
        header = *reinterpret_cast<BSSectionHeader*>(data + offset);
        if (header.size > sizeof(structure)) {
            structure = reinterpret_cast<BSSectionHeader*>(
                data + offset + sizeof(BSSectionHeader));
            if (structure->id != SID_Struct) {
                structure = nullptr;
            }
        }
    }

    template <class T>
    T readStructure() {
        return *reinterpret_cast<T*>(data + offset +
                                     sizeof(BSSectionHeader) * 2);
    }

    template <class T>
    T& readSubStructure(size_t internalOffset) {
        return *reinterpret_cast<T*>(data + offset + sizeof(BSSectionHeader) +
                                     internalOffset);
    }

    template <class T>
    T readRaw(size_t internalOffset) {
        return *reinterpret_cast<T*>(data + offset + internalOffset);
    }

    char* raw() {
        return data + offset + sizeof(BSSectionHeader);
    }

    bool hasMoreData(size_t length) {
        return (length) < (header.size);
    }

    BinaryStreamSection getNextChildSection(size_t& internalOffset) {
        size_t realOffset = internalOffset;
        RW_ASSERT(realOffset < header.size);
        BinaryStreamSection sec(data,
                                offset + sizeof(BSSectionHeader) + realOffset);
        internalOffset += sec.header.size + sizeof(BSSectionHeader);
        return sec;
    }
};
}

#endif
