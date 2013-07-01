#include "LoaderDFF.h"

#include "../framework/rwbinarystream.h"

#include <iostream>

void LoaderDFF::loadFromMemory(char *data)
{
	RW::BinaryStreamSection root(data);

	this->clump = root.readStructure<RW::BSClump>();

	size_t dataI = 0;
	while (root.hasMoreData(dataI)) {
		auto sec = root.getNextChildSection(dataI);

		switch (sec.header.id) {
		case RW::SID_GeometryList: {
			auto list = sec.readStructure<RW::BSGeometryList>();
			size_t gdataI = 0;
			while (sec.hasMoreData(gdataI)) {
				Geometry geometryStruct;
				auto item = sec.getNextChildSection(gdataI);

				if (item.header.id == RW::SID_Geometry) {
					size_t dataI = 0, secI = 0;
					auto geometry = item.readStructure<RW::BSGeometry>();
					std::cout << " verts(" << geometry.numverts << ") tris(" << geometry.numtris << ")" << std::endl;
					item.getNextChildSection(secI);
					char *data = item.raw() + sizeof(RW::BSSectionHeader) + sizeof(RW::BSGeometry);

					if (item.header.versionid < 0x1003FFFF)
						auto colors = readStructure<RW::BSGeometryColor>(data, dataI);

					if (geometry.flags & RW::BSGeometry::VertexColors) {
						for (size_t v = 0; v < geometry.numverts; ++v) {
							readStructure<RW::BSColor>(data, dataI);
						}
					}

					/** TEX COORDS **/
					if (geometry.flags & RW::BSGeometry::TexCoords1 || geometry.flags & RW::BSGeometry::TexCoords2) {
						for (size_t v = 0; v < geometry.numverts; ++v) {
							geometryStruct.texcoords.push_back(readStructure<RW::BSGeometryUV>(data, dataI));
						}
					}

					/** INDICIES **/
					for (int j = 0; j < geometry.numtris; ++j) {
						geometryStruct.triangles.push_back(readStructure<RW::BSGeometryTriangle>(data, dataI));
					}

					/** GEOMETRY BOUNDS **/
					geometryStruct.geometryBounds = readStructure<RW::BSGeometryBounds>(data, dataI);

					/** VERTICES **/
					for (int v = 0; v < geometry.numverts; ++v) {
						geometryStruct.vertices.push_back(readStructure<RW::BSTVector3>(data, dataI));
					}

					/** NORMALS **/
					if (geometry.flags & RW::BSGeometry::StoreNormals) {
						for (int n = 0; n < geometry.numverts; ++n) {
							geometryStruct.normals.push_back(readStructure<RW::BSTVector3>(data, dataI));
						}
					}

					/** TEXTURES **/
					auto materiallistsec = item.getNextChildSection(secI);
					auto materialList = materiallistsec.readStructure<RW::BSMaterialList>();

					// Skip over the per-material byte values that I don't know what do.
					dataI += sizeof(uint32_t) * materialList.nummaterials;

					size_t matI = 0;
					materiallistsec.getNextChildSection(matI);

					for (size_t m = 0; m < materialList.nummaterials; ++m) {
						auto materialsec = materiallistsec.getNextChildSection(matI);
						if (materialsec.header.id != RW::SID_Material)
							continue;

						auto material = materialsec.readStructure<RW::BSMaterial>();

						size_t texI = 0;
						materialsec.getNextChildSection(texI);

						for (size_t t = 0; t < material.numtextures; ++t) {
							auto texsec = materialsec.getNextChildSection(texI);
							auto texture = texsec.readStructure<RW::BSTexture>();

							std::string textureName, alphaName;
							size_t yetAnotherI = 0;
							texsec.getNextChildSection(yetAnotherI);

							auto namesec = texsec.getNextChildSection(yetAnotherI);
							auto alphasec = texsec.getNextChildSection(yetAnotherI);

							// The data is null terminated anyway.
							textureName = namesec.raw();
							alphaName = alphasec.raw();

							geometryStruct.textures.push_back({textureName, alphaName});
						}
					}

					// Add it
					geometries.push_back(geometryStruct);
				}
			}
		}
		}
	}
}

template<class T> T LoaderDFF::readStructure(char *data, size_t &dataI)
{
	size_t originalOffset = dataI;
	dataI += sizeof(T);
	return *reinterpret_cast<T*>(data + originalOffset);
}

RW::BSSectionHeader LoaderDFF::readHeader(char *data, size_t &dataI)
{
	return readStructure<RW::BSSectionHeader>(data, dataI);
}
