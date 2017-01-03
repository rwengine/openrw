#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#include <gl/TextureData.hpp>
#include <loaders/RWBinaryStream.hpp>
#include <platform/FileHandle.hpp>

#include <functional>
#include <string>

class Clump;

class DFFLoaderException {
    std::string _message;

public:
    DFFLoaderException(const std::string& message) : _message(message) {
    }

    const std::string& which() {
        return _message;
    }
};

class LoaderDFF {
    /**
     * @brief loads a Frame List chunk from stream into model.
     * @param model
     * @param stream
     */
    void readFrameList(Clump* model, const RWBStream& stream);

    void readGeometryList(Clump* model, const RWBStream& stream);

    void readGeometry(Clump* model, const RWBStream& stream);

    void readMaterialList(Clump* model, const RWBStream& stream);

    void readMaterial(Clump* model, const RWBStream& stream);

    void readTexture(Clump* model, const RWBStream& stream);

    void readGeometryExtension(Clump* model, const RWBStream& stream);

    void readBinMeshPLG(Clump* model, const RWBStream& stream);

    void readAtomic(Clump* model, const RWBStream& stream);

public:
    using TextureLookupCallback = std::function<TextureData::Handle(
        const std::string&, const std::string&)>;

    Clump* loadFromMemory(FileHandle file);

    void setTextureLookupCallback(TextureLookupCallback tlc) {
        texturelookup = tlc;
    }
private:
    TextureLookupCallback texturelookup;
};

#endif
