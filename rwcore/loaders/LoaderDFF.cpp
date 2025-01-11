#include "loaders/LoaderDFF.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <numeric>

#include <glm/glm.hpp>

#include "data/Clump.hpp"
#include "gl/gl_core_3_3.h"
#include "loaders/RWBinaryStream.hpp"
#include "platform/FileHandle.hpp"
#include "rw/debug.hpp"

enum DFFChunks {
    CHUNK_STRUCT = 0x0001,
    CHUNK_EXTENSION = 0x0003,
    CHUNK_TEXTURE = 0x0006,
    CHUNK_MATERIAL = 0x0007,
    CHUNK_MATERIALLIST = 0x0008,
    CHUNK_FRAMELIST = 0x000E,
    CHUNK_GEOMETRY = 0x000F,
    CHUNK_CLUMP = 0x0010,

    CHUNK_ATOMIC = 0x0014,

    CHUNK_GEOMETRYLIST = 0x001A,

    CHUNK_BINMESHPLG = 0x050E,

    CHUNK_NODENAME = 0x0253F2FE,
};

// These structs are used to interpret raw bytes from the stream.
/// @todo worry about endianness.

typedef glm::vec3 BSTVector3;
typedef glm::mat3 BSTMatrix;
typedef glm::i8vec4 BSTColour;

struct RWBSFrame {
    BSTMatrix rotation;
    BSTVector3 position;
    int32_t index;
    uint32_t matrixflags;  // Not used
};

LoaderDFF::FrameList LoaderDFF::readFrameList(const RWBStream &stream) {
    auto listStream = stream.getInnerStream();

    auto listStructID = listStream.getNextChunk();
    if (listStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Frame List missing struct chunk");
    }

    char *headerPtr = listStream.getCursor();

    unsigned int numFrames = *reinterpret_cast<std::uint32_t *>(headerPtr);
    headerPtr += sizeof(std::uint32_t);

    FrameList framelist;
    framelist.reserve(numFrames);

    for (auto f = 0u; f < numFrames; ++f) {
        auto data = reinterpret_cast<RWBSFrame *>(headerPtr);
        headerPtr += sizeof(RWBSFrame);
        auto frame =
            std::make_shared<ModelFrame>(f, data->rotation, data->position);

        RW_CHECK(data->index < static_cast<int>(framelist.size()),
                 "Frame parent out of bounds");
        if (data->index != -1 &&
            data->index < static_cast<int>(framelist.size())) {
            framelist[data->index]->addChild(frame);
        }

        framelist.push_back(frame);
    }

    size_t namedFrames = 0;

    /// @todo perhaps flatten this out a little
    for (auto chunkID = listStream.getNextChunk(); chunkID != 0;
         chunkID = listStream.getNextChunk()) {
        switch (chunkID) {
            case CHUNK_EXTENSION: {
                auto extStream = listStream.getInnerStream();
                for (auto chunkID = extStream.getNextChunk(); chunkID != 0;
                     chunkID = extStream.getNextChunk()) {
                    switch (chunkID) {
                        case CHUNK_NODENAME: {
                            std::string fname(extStream.getCursor(),
                                              extStream.getCurrentChunkSize());
                            std::transform(fname.begin(), fname.end(),
                                           fname.begin(), ::tolower);

                            if (namedFrames < framelist.size()) {
                                framelist[namedFrames++]->setName(fname);
                            }
                        } break;
                        default:
                            break;
                    }
                }
            } break;
            default:
                break;
        }
    }

    return framelist;
}

LoaderDFF::GeometryList LoaderDFF::readGeometryList(const RWBStream &stream) {
    auto listStream = stream.getInnerStream();

    auto listStructID = listStream.getNextChunk();
    if (listStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Geometry List missing struct chunk");
    }

    char *headerPtr = listStream.getCursor();

    unsigned int numGeometries = bit_cast<std::uint32_t>(*headerPtr);
    headerPtr += sizeof(std::uint32_t);

    std::vector<GeometryPtr> geometrylist;
    geometrylist.reserve(numGeometries);

    for (auto chunkID = listStream.getNextChunk(); chunkID != 0;
         chunkID = listStream.getNextChunk()) {
        switch (chunkID) {
            case CHUNK_GEOMETRY: {
                geometrylist.push_back(readGeometry(listStream));
            } break;
            default:
                break;
        }
    }

    return geometrylist;
}

GeometryPtr LoaderDFF::readGeometry(const RWBStream &stream) {
    auto geomStream = stream.getInnerStream();

    auto geomStructID = geomStream.getNextChunk();
    if (geomStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Geometry missing struct chunk");
    }

    auto geom = std::make_shared<Geometry>();

    char *headerPtr = geomStream.getCursor();

    geom->flags = bit_cast<std::uint16_t>(*headerPtr);
    headerPtr += sizeof(std::uint16_t);

    /*unsigned short numUVs = bit_cast<std::uint8_t>(*headerPtr);*/
    headerPtr += sizeof(std::uint8_t);
    /*unsigned short moreFlags = bit_cast<std::uint8_t>(*headerPtr);*/
    headerPtr += sizeof(std::uint8_t);

    unsigned int numTris = bit_cast<std::uint32_t>(*headerPtr);
    headerPtr += sizeof(std::uint32_t);
    unsigned int numVerts = bit_cast<std::uint32_t>(*headerPtr);
    headerPtr += sizeof(std::uint32_t);

    /*unsigned int numFrames = bit_cast<std::uint32_t>(*headerPtr);*/
    headerPtr += sizeof(std::uint32_t);

    std::vector<GeometryVertex> verts;
    verts.resize(numVerts);

    if (geomStream.getChunkVersion() < 0x34003) {
        headerPtr += sizeof(RW::BSGeometryColor);
    }

    /// @todo extract magic numbers.

    if ((geom->flags & 8) == 8) {
        for (size_t v = 0; v < numVerts; ++v) {
            verts[v].colour = bit_cast<glm::u8vec4>(*headerPtr);
            headerPtr += sizeof(glm::u8vec4);
        }
    } else {
        for (size_t v = 0; v < numVerts; ++v) {
            verts[v].colour = {255, 255, 255, 255};
        }
    }

    if ((geom->flags & 4) == 4 || (geom->flags & 128) == 128) {
        for (size_t v = 0; v < numVerts; ++v) {
            verts[v].texcoord = bit_cast<glm::vec2>(*headerPtr);
            headerPtr += sizeof(glm::vec2);
        }
    }

    // Grab indicies data to generate normals (if applicable).
    auto triangles = std::make_unique<RW::BSGeometryTriangle[]>(numTris);
    memcpy(triangles.get(), headerPtr, sizeof(RW::BSGeometryTriangle) * numTris);
    headerPtr += sizeof(RW::BSGeometryTriangle) * numTris;

    geom->geometryBounds = bit_cast<RW::BSGeometryBounds>(*headerPtr);
    geom->geometryBounds.radius = std::abs(geom->geometryBounds.radius);
    headerPtr += sizeof(RW::BSGeometryBounds);

    for (size_t v = 0; v < numVerts; ++v) {
        verts[v].position = bit_cast<glm::vec3>(*headerPtr);
        headerPtr += sizeof(glm::vec3);
    }

    if ((geom->flags & 16) == 16) {
        for (size_t v = 0; v < numVerts; ++v) {
            verts[v].normal = bit_cast<glm::vec3>(*headerPtr);
            headerPtr += sizeof(glm::vec3);
        }
    } else {
        // Use triangle data to calculate normals for each vert.
        for (size_t t = 0; t < numTris; ++t) {
            auto &triangle = triangles[t];
            auto &A = verts[triangle.first];
            auto &B = verts[triangle.second];
            auto &C = verts[triangle.third];
            auto normal = glm::normalize(
                glm::cross(C.position - A.position, B.position - A.position));
            A.normal = normal;
            B.normal = normal;
            C.normal = normal;
        }
    }

    // Process the geometry child sections
    for (auto chunkID = geomStream.getNextChunk(); chunkID != 0;
         chunkID = geomStream.getNextChunk()) {
        switch (chunkID) {
            case CHUNK_MATERIALLIST:
                readMaterialList(geom, geomStream);
                break;
            case CHUNK_EXTENSION:
                readGeometryExtension(geom, geomStream);
                break;
            default:
                break;
        }
    }

    geom->dbuff.setFaceType(geom->facetype == Geometry::Triangles
                                ? GL_TRIANGLES
                                : GL_TRIANGLE_STRIP);
    geom->gbuff.uploadVertices(verts);
    geom->dbuff.addGeometry(&geom->gbuff);

    glGenBuffers(1, &geom->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->EBO);

    size_t icount = std::accumulate(
        geom->subgeom.begin(), geom->subgeom.end(), size_t{0u},
        [](size_t a, const SubGeometry &b) { return a + b.numIndices; });
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * icount, nullptr,
                 GL_STATIC_DRAW);
    for (auto &sg : geom->subgeom) {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sg.start * sizeof(uint32_t),
                        sizeof(uint32_t) * sg.numIndices, sg.indices.data());
    }

    return geom;
}

void LoaderDFF::readMaterialList(const GeometryPtr &geom, const RWBStream &stream) {
    auto listStream = stream.getInnerStream();

    auto listStructID = listStream.getNextChunk();
    if (listStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("MaterialList missing struct chunk");
    }

    unsigned int numMaterials = bit_cast<std::uint32_t>(*listStream.getCursor());

    geom->materials.reserve(numMaterials);

    RWBStream::ChunkID chunkID;
    while ((chunkID = listStream.getNextChunk())) {
        switch (chunkID) {
            case CHUNK_MATERIAL:
                readMaterial(geom, listStream);
                break;
            default:
                break;
        }
    }
}

void LoaderDFF::readMaterial(const GeometryPtr &geom, const RWBStream &stream) {
    auto materialStream = stream.getInnerStream();

    auto matStructID = materialStream.getNextChunk();
    if (matStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Material missing struct chunk");
    }

    char *matData = materialStream.getCursor();

    Geometry::Material material;

    // Unkown
    matData += sizeof(std::uint32_t);
    material.colour = bit_cast<glm::u8vec4>(*matData);

    matData += sizeof(std::uint32_t);
    // Unkown
    matData += sizeof(std::uint32_t);
    /*bool usesTexture = bit_cast<std::uint32_t>(*matData);*/
    matData += sizeof(std::uint32_t);

    material.ambientIntensity = bit_cast<float>(*matData);
    matData += sizeof(float);

    /*float specular = bit_cast<float>(*matData);*/
    matData += sizeof(float);

    material.diffuseIntensity = bit_cast<float>(*matData);
    matData += sizeof(float);
    material.flags = 0;

    RWBStream::ChunkID chunkID;
    while ((chunkID = materialStream.getNextChunk())) {
        switch (chunkID) {
            case CHUNK_TEXTURE:
                readTexture(material, materialStream);
                break;
            default:
                break;
        }
    }

    geom->materials.push_back(material);
}

void LoaderDFF::readTexture(Geometry::Material &material,
                            const RWBStream &stream) {
    auto texStream = stream.getInnerStream();

    auto texStructID = texStream.getNextChunk();
    if (texStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Texture missing struct chunk");
    }

    // There's some data in the Texture's struct, but we don't know what it is.

    /// @todo improve how these strings are read.
    std::string name, alpha;

    texStream.getNextChunk();
    name = texStream.getCursor();
    texStream.getNextChunk();
    alpha = texStream.getCursor();

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    std::transform(alpha.begin(), alpha.end(), alpha.begin(), ::tolower);

    auto textureInstPtr = textureLookup ? textureLookup(name, alpha) : nullptr;
    material.textures.emplace_back(std::move(name), std::move(alpha), textureInstPtr);
}

void LoaderDFF::readGeometryExtension(const GeometryPtr &geom,
                                      const RWBStream &stream) {
    auto extStream = stream.getInnerStream();

    RWBStream::ChunkID chunkID;
    while ((chunkID = extStream.getNextChunk())) {
        switch (chunkID) {
            case CHUNK_BINMESHPLG:
                readBinMeshPLG(geom, extStream);
                break;
            default:
                break;
        }
    }
}

void LoaderDFF::readBinMeshPLG(const GeometryPtr &geom, const RWBStream &stream) {
    auto data = stream.getCursor();

    geom->facetype = static_cast<Geometry::FaceType>(bit_cast<std::uint32_t>(*data));
    data += sizeof(std::uint32_t);

    unsigned int numSplits = bit_cast<std::uint32_t>(*data);
    data += sizeof(std::uint32_t);

    // Number of triangles.
    data += sizeof(std::uint32_t);

    geom->subgeom.reserve(numSplits);

    size_t start = 0;

    for (size_t s = 0; s < numSplits; ++s) {
        SubGeometry sg;
        sg.numIndices = bit_cast<std::uint32_t>(*data);
        data += sizeof(std::uint32_t);
        sg.material = bit_cast<std::uint32_t>(*data);
        data += sizeof(std::uint32_t);
        sg.start = start;
        start += sg.numIndices;

        sg.indices.resize(sg.numIndices);
        std::memcpy(sg.indices.data(), data,
                    sizeof(std::uint32_t) * sg.numIndices);
        data += sizeof(std::uint32_t) * sg.numIndices;

        geom->subgeom.push_back(std::move(sg));
    }
}

AtomicPtr LoaderDFF::readAtomic(FrameList &framelist,
                                GeometryList &geometrylist,
                                const RWBStream &stream) {
    auto atomicStream = stream.getInnerStream();

    auto atomicStructID = atomicStream.getNextChunk();
    if (atomicStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Atomic missing struct chunk");
    }

    auto data = atomicStream.getCursor();
    std::uint32_t frame = bit_cast<std::uint32_t>(*data);
    data += sizeof(std::uint32_t);

    std::uint32_t geometry = bit_cast<std::uint32_t>(*data);
    data += sizeof(std::uint32_t);

    std::uint32_t flags = bit_cast<std::uint32_t>(*data);

    // Verify the atomic's particulars
    RW_CHECK(frame < framelist.size(), "atomic frame " << frame
                                                       << " out of bounds");
    RW_CHECK(geometry < geometrylist.size(),
             "atomic geometry " << geometry << " out of bounds");

    auto atomic = std::make_shared<Atomic>();
    if (geometry < geometrylist.size()) {
        atomic->setGeometry(geometrylist[geometry]);
    }
    if (frame < framelist.size()) {
        atomic->setFrame(framelist[frame]);
    }
    atomic->setFlags(flags);

    return atomic;
}

ClumpPtr LoaderDFF::loadFromMemory(const FileContentsInfo& file) {
    auto model = std::make_shared<Clump>();

    RWBStream rootStream(file.data.get(), file.length);

    auto rootID = rootStream.getNextChunk();
    if (rootID != CHUNK_CLUMP) {
        throw DFFLoaderException("Invalid root section ID " +
                                 std::to_string(rootID));
    }

    RWBStream modelStream = rootStream.getInnerStream();
    auto rootStructID = modelStream.getNextChunk();
    if (rootStructID != CHUNK_STRUCT) {
        throw DFFLoaderException("Clump missing struct chunk");
    }

    // There is only one value in the struct section.
    std::uint32_t numAtomics = bit_cast<std::uint32_t>(*rootStream.getCursor());
    RW_UNUSED(numAtomics);

    GeometryList geometrylist;
    FrameList framelist;

    // Process everything inside the clump stream.
    RWBStream::ChunkID chunkID;
    while ((chunkID = modelStream.getNextChunk())) {
        switch (chunkID) {
            case CHUNK_FRAMELIST:
                framelist = readFrameList(modelStream);
                break;
            case CHUNK_GEOMETRYLIST:
                geometrylist = readGeometryList(modelStream);
                break;
            case CHUNK_ATOMIC: {
                auto atomic = readAtomic(framelist, geometrylist, modelStream);
                RW_CHECK(atomic, "Failed to read atomic");
                if (!atomic) {
                    // Abort reading the rest of the clump
                    return nullptr;
                }
                model->addAtomic(atomic);
            } break;
            default:
                break;
        }
    }

    if (!framelist.empty()) {
        model->setFrame(framelist[0]);
    }

    // Ensure the model has cached metrics
    model->recalculateMetrics();

    return model;
}
