#include "loaders/LoaderCOL.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>

#include <glm/glm.hpp>

#include "data/CollisionModel.hpp"
#include "rw/bit_cast.cpp"

constexpr uint32_t kCollMagic = 0x4C4C4F43;

struct ColHeader {
    uint32_t ident;
    uint32_t size;
    char name[22];
    uint16_t modelid;
};

bool LoaderCOL::load(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios_base::binary);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios_base::end);
    size_t length = file.tellg();
    file.seekg(0);

    std::vector<char> buffer(length);
    auto d = buffer.data();
    file.read(d, length);

    while (d < buffer.data() + length) {
        ColHeader head;
        std::memcpy(&head, d, sizeof(head));
        d += sizeof(head);

        if (head.ident != kCollMagic) {
            throw std::runtime_error("Bad magic " + std::to_string(head.ident) +
                                     " in " + path);
        }

        auto model = std::make_unique<CollisionModel>();
        model->name = head.name;
        model->modelid = head.modelid;

        auto readFloat = [&]() {
            auto f = bit_cast<float>(*d);
            d += sizeof(float);
            return f;
        };
        auto readU8 = [&]() {
            auto f = bit_cast<uint8_t>(*d);
            d += sizeof(uint8_t);
            return f;
        };
        auto readU32 = [&]() {
            auto f = bit_cast<uint32_t>(*d);
            d += sizeof(uint32_t);
            return f;
        };
        auto readVec3 = [&]() {
            auto x = readFloat();
            auto y = readFloat();
            auto z = readFloat();
            return glm::vec3(x, y, z);
        };
        auto readSurface = [&]() {
            return CollisionModel::Surface{readU8(), readU8(), readU8(),
                                           readU8()};
        };

        model->boundingSphere.radius = readFloat();
        model->boundingSphere.center = readVec3();
        model->boundingBox.min = readVec3();
        model->boundingBox.max = readVec3();

        // Read Spheres
        auto numspheres = readU32();
        model->spheres.resize(numspheres);
        for (auto& sphere : model->spheres) {
            sphere.radius = readFloat();
            sphere.center = readVec3();
            sphere.surface = readSurface();
        }

        auto numlines = readU32();
        // Ignore lines, we don't support them
        if (numlines > 0) {
            throw std::runtime_error("Unuspported line type in " + path);
        }

        // Read boxes
        auto numboxes = readU32();
        model->boxes.resize(numboxes);
        for (auto& box : model->boxes) {
            box.min = readVec3();
            box.max = readVec3();
            box.surface = readSurface();
        }

        // Read mesh
        auto numvertices = readU32();
        model->vertices.resize(numvertices);
        for (auto& v : model->vertices) {
            v = readVec3();
        }

        auto numtriangles = readU32();
        model->faces.resize(numtriangles);
        for (auto& t : model->faces) {
            t.tri[0] = readU32();
            t.tri[1] = readU32();
            t.tri[2] = readU32();
            t.surface = readSurface();
        }

        collisions.emplace_back(std::move(model));
    }

    return true;
}
