#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#include <data/Clump.hpp>
#include <gl/TextureData.hpp>
#include <loaders/RWBinaryStream.hpp>
#include <platform/FileHandle.hpp>

#include <functional>
#include <string>

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
public:
    using TextureLookupCallback = std::function<TextureData::Handle(
        const std::string&, const std::string&)>;
    using GeometryList = std::vector<GeometryPtr>;
    using FrameList = std::vector<ModelFramePtr>;

    Clump* loadFromMemory(FileHandle file);

    void setTextureLookupCallback(TextureLookupCallback tlc) {
        texturelookup = tlc;
    }

private:
    TextureLookupCallback texturelookup;

    FrameList readFrameList(const RWBStream& stream);

    GeometryList readGeometryList(const RWBStream& stream);

    GeometryPtr readGeometry(const RWBStream& stream);

    void readMaterialList(GeometryPtr& geom, const RWBStream& stream);

    void readMaterial(GeometryPtr& geom, const RWBStream& stream);

    void readTexture(Geometry::Material& material, const RWBStream& stream);

    void readGeometryExtension(GeometryPtr& geom, const RWBStream& stream);

    void readBinMeshPLG(GeometryPtr& geom, const RWBStream& stream);

    AtomicPtr readAtomic(FrameList& framelist, GeometryList& geometrylist,
                         const RWBStream& stream);
};

#endif
