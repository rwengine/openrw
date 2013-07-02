#include "loaders/LoaderDFF.h"

#include <iostream>

std::unique_ptr<Model> LoaderDFF::loadFromMemory(char *data)
{
	auto model = std::unique_ptr<Model>(new Model);
	RW::BinaryStreamSection root(data);

	model->clump = root.readStructure<RW::BSClump>();

	size_t dataI = 0;
	while (root.hasMoreData(dataI)) {
		auto sec = root.getNextChildSection(dataI);

		switch (sec.header.id) {
		case RW::SID_GeometryList: {
			auto list = sec.readStructure<RW::BSGeometryList>();
			size_t gdataI = 0;
			while (sec.hasMoreData(gdataI)) {
				Model::Geometry geometryStruct;
				auto item = sec.getNextChildSection(gdataI);

				if (item.header.id == RW::SID_Geometry) {
					size_t dataI = 0, secI = 0;
					auto geometry = item.readStructure<RW::BSGeometry>();
					// std::cout << " verts(" << geometry.numverts << ") tris(" << geometry.numtris << ")" << std::endl;

					item.getNextChildSection(secI);
					char *data = item.raw() + sizeof(RW::BSSectionHeader) + sizeof(RW::BSGeometry);

					if (item.header.versionid < 0x1003FFFF)
						auto colors = readStructure<RW::BSGeometryColor>(data, dataI);
					
					if ((geometry.flags & RW::BSGeometry::VertexColors) == RW::BSGeometry::VertexColors) {
						for (size_t v = 0; v < geometry.numverts; ++v) {
							readStructure<RW::BSColor>(data, dataI);
						}
					}

					/** TEX COORDS **/
					if ((geometry.flags & RW::BSGeometry::TexCoords1) == RW::BSGeometry::TexCoords1 || 
						(geometry.flags & RW::BSGeometry::TexCoords2) == RW::BSGeometry::TexCoords1) {
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
					if ((geometry.flags & RW::BSGeometry::StoreNormals) == RW::BSGeometry::StoreNormals) {
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
					
					geometryStruct.materials.resize(materialList.nummaterials);

					for (size_t m = 0; m < materialList.nummaterials; ++m) {
						auto materialsec = materiallistsec.getNextChildSection(matI);
						if (materialsec.header.id != RW::SID_Material)
							continue;

						auto material = materialsec.readStructure<RW::BSMaterial>();
						geometryStruct.materials[m].textures.resize(material.numtextures);

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

							geometryStruct.materials[m].textures[t] = {textureName, alphaName};
						}
					}
					
					if(item.hasMoreData(secI))
					{
						auto extensions = item.getNextChildSection(secI);
						size_t extI = 0;
						while(extensions.hasMoreData(extI))
						{
							auto extsec = extensions.getNextChildSection(extI);
							if(extsec.header.id == RW::SID_BinMeshPLG)
							{
								auto meshplg = extsec.readSubStructure<RW::BSBinMeshPLG>(0);
								geometryStruct.subgeom.resize(meshplg.numsplits);
								size_t meshplgI = sizeof(RW::BSBinMeshPLG);
								for(size_t i = 0; i < meshplg.numsplits; ++i)
								{
									auto plgHeader = extsec.readSubStructure<RW::BSMaterialSplit>(meshplgI);
									meshplgI += sizeof(RW::BSMaterialSplit);
									geometryStruct.subgeom[i].material = plgHeader.index;
									geometryStruct.subgeom[i].indices.resize(plgHeader.numverts);
									for (int j = 0; j < plgHeader.numverts; ++j) {
										geometryStruct.subgeom[i].indices[j] = extsec.readSubStructure<uint32_t>(meshplgI);
										meshplgI += sizeof(uint32_t);
									}
								}
							}
						}
					}

					// OpenGL buffer stuff
					glGenBuffers(1, &geometryStruct.VBO);
					glGenBuffers(1, &geometryStruct.EBO);
					for(size_t i  = 0; i < geometryStruct.subgeom.size(); ++i)
					{
						glGenBuffers(1, &(geometryStruct.subgeom[i].EBO));
					}

					size_t buffsize = (geometryStruct.vertices.size() * sizeof(float) * 3)
									+ (geometryStruct.texcoords.size() * sizeof(float) * 2)
									+ (geometryStruct.normals.size() * sizeof(float) * 3);
					
					// Vertices
					glBindBuffer(GL_ARRAY_BUFFER, geometryStruct.VBO);
					glBufferData(GL_ARRAY_BUFFER, buffsize, NULL, GL_STATIC_DRAW);
					
					glBufferSubData(
						GL_ARRAY_BUFFER,
						0,
						(geometryStruct.vertices.size() * sizeof(float) * 3),
						&geometryStruct.vertices[0]
					);
					
					if(geometryStruct.texcoords.size() > 0)
					{
						glBufferSubData(
							GL_ARRAY_BUFFER,
							(geometryStruct.vertices.size() * sizeof(float) * 3),
							(geometryStruct.texcoords.size() * sizeof(float) * 2),
							&geometryStruct.texcoords[0]
						);
					}
					
					if(geometryStruct.normals.size() > 0 )
					{
						glBufferSubData(
							GL_ARRAY_BUFFER,
							(geometryStruct.vertices.size() * sizeof(float) * 3) + (geometryStruct.texcoords.size() * sizeof(float) * 2),
							geometryStruct.normals.size() * 3 * sizeof(float),
							&geometryStruct.normals[0]
						);
					}

					// Elements
					uint16_t indicies[geometryStruct.triangles.size() * 3];
					size_t i = 0;
					for (auto &tri : geometryStruct.triangles) {
						indicies[i]     = tri.first;
						indicies[i + 1] = tri.second;
						indicies[i + 2] = tri.third;
						i += 3;
					}
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryStruct.EBO);
					glBufferData(
						GL_ELEMENT_ARRAY_BUFFER,
						sizeof(indicies),
						indicies,
						GL_STATIC_DRAW
					);
					
					for(size_t i  = 0; i < geometryStruct.subgeom.size(); ++i)
					{
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryStruct.subgeom[i].EBO);
						glBufferData(
							GL_ELEMENT_ARRAY_BUFFER,
							sizeof(uint32_t) * geometryStruct.subgeom[i].indices.size(),
							&(geometryStruct.subgeom[i].indices[0]),
							GL_STATIC_DRAW
						);
					}
					
					// Add it
					model->geometries.push_back(geometryStruct);
					
					
				}
			}
		}
		}
	}

	return model;
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
