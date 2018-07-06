#ifndef _LIBRW_LOADERDFF_HPP_
#define _LIBRW_LOADERDFF_HPP_

#include <data/Clump.hpp>
#include <gl/TextureData.hpp>
#include <rw/forward.hpp>

#include <functional>
#include <string>
#include <vector>

class RWBStream;

class DFFLoaderException {
    std::string _message;

public:
    template <class String>
    DFFLoaderException(String&& message) : _message(message) {
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

    ClumpPtr loadFromMemory(const FileContentsInfo& file);

    void setTextureLookupCallback(const TextureLookupCallback& tlc) {
        texturelookup = tlc;
    }

private:
    TextureLookupCallback texturelookup;

    FrameList readFrameList(const RWBStream& stream);

    GeometryList readGeometryList(const RWBStream& stream);

    GeometryPtr readGeometry(const RWBStream& stream);

    void readMaterialList(const GeometryPtr& geom, const RWBStream& stream);

    void readMaterial(const GeometryPtr& geom, const RWBStream& stream);

    void readTexture(Geometry::Material& material, const RWBStream& stream);

    void readGeometryExtension(const GeometryPtr& geom, const RWBStream& stream);

    void readBinMeshPLG(const GeometryPtr& geom, const RWBStream& stream);

    AtomicPtr readAtomic(FrameList& framelist, GeometryList& geometrylist,
                         const RWBStream& stream);
};

#endif
