#include <loaders/LoaderDFF.hpp>
#include <engine/GameData.hpp>
#include <render/Model.hpp>

#include <iostream>
#include <algorithm>
#include <set>
#include <cstring>

Model* LoaderDFF::loadFromMemory(char *data, GameData *gameData)
{
    auto model = new Model;
	RW::BinaryStreamSection root(data);

	model->clump = root.readStructure<RW::BSClump>();

	size_t dataI = 0, clumpID = 0;
	while (root.hasMoreData(dataI)) {
		auto sec = root.getNextChildSection(dataI);

		switch (sec.header.id) {
		case RW::SID_FrameList: {
			auto list = sec.readStructure<RW::BSFrameList>();
			size_t fdataI = sizeof(RW::BSFrameList) + sizeof(RW::BSSectionHeader);
			
			model->frames.reserve(list.numframes);
			
			for(size_t f = 0; f < list.numframes; ++f) {
                RW::BSFrameListFrame& rawframe = sec.readSubStructure<RW::BSFrameListFrame>(fdataI);
				fdataI += sizeof(RW::BSFrameListFrame);
				ModelFrame* parent = nullptr;
				if(rawframe.index != -1) {
					parent = model->frames[rawframe.index];
				}
				else {
					model->rootFrameIdx = 0;
				}
				model->frames.push_back(
					new ModelFrame(parent, rawframe.rotation, rawframe.position)
				);
			}
			
			size_t fldataI = 0;
			while( sec.hasMoreData(fldataI)) {
				auto listsec = sec.getNextChildSection(fldataI);
				if( listsec.header.id == RW::SID_Extension) {
					size_t extI = 0;
					while( listsec.hasMoreData(extI)) {
						auto extSec = listsec.getNextChildSection(extI);
						size_t fn = 0;
						if( extSec.header.id == RW::SID_NodeName) {
                            std::string framename(extSec.raw(), extSec.header.size);
                            std::transform(framename.begin(), framename.end(), framename.begin(), ::tolower );

							// !HACK!
							if(framename == "swaist") {
								model->rootFrameIdx = model->frameNames.size();
							}
							
							if( fn < model->frames.size() ) {
								model->frames[(fn++)]->setName(framename);
							}
						}
					}
				}
			}
			
			break;
		}
		case RW::SID_GeometryList: {
			/*auto list =*/ sec.readStructure<RW::BSGeometryList>();
			size_t gdataI = 0;
			while (sec.hasMoreData(gdataI)) {
				std::shared_ptr<Model::Geometry> geom(new Model::Geometry);
				
				auto item = sec.getNextChildSection(gdataI);

				if (item.header.id == RW::SID_Geometry) {
					size_t dataI = 0, secI = 0;
					auto geometry = item.readStructure<RW::BSGeometry>();
					// std::cout << " verts(" << geometry.numverts << ") tris(" << geometry.numtris << ")" << std::endl;
					
					geom->flags = geometry.flags;

					item.getNextChildSection(secI);
					char *data = item.raw() + sizeof(RW::BSSectionHeader) + sizeof(RW::BSGeometry);

					if (item.header.versionid < 0x1003FFFF)
						/*auto colors =*/ readStructure<RW::BSGeometryColor>(data, dataI);
					
					std::vector<glm::vec4> colours;
					colours.resize(geometry.numverts);
					if ((geometry.flags & RW::BSGeometry::VertexColors) == RW::BSGeometry::VertexColors) {
						for (size_t v = 0; v < geometry.numverts; ++v) {
							auto s = readStructure<RW::BSColor>(data, dataI);
							size_t R = s % 256; s /= 256;
							size_t G = s % 256; s /= 256;
							size_t B = s % 256; s /= 256;
							size_t A = s % 256;
							colours[v] = glm::vec4(R/255.f, G/255.f, B/255.f, A/255.f);
						}
					}
					else {
						// To save needing another shader, just insert a white colour for each vertex
						for (size_t v = 0; v < geometry.numverts; ++v) {
							colours[v] = glm::vec4(1.f);
						}
					}
					geom->setColours(colours);
					
					/** TEX COORDS **/
					if ((geometry.flags & RW::BSGeometry::TexCoords1) == RW::BSGeometry::TexCoords1 || 
						(geometry.flags & RW::BSGeometry::TexCoords2) == RW::BSGeometry::TexCoords1) {
						std::vector<glm::vec2> texcoords;
						texcoords.resize(geometry.numverts);
						for (size_t v = 0; v < geometry.numverts; ++v) {
							texcoords[v] = readStructure<glm::vec2>(data, dataI);
						}
						geom->setTexCoords(texcoords);
					}

					//geometryStruct.triangles.reserve(geometry.numtris);
					for (int j = 0; j < geometry.numtris; ++j) {
						readStructure<RW::BSGeometryTriangle>(data, dataI);
					}

					/** GEOMETRY BOUNDS **/
					geom->geometryBounds = readStructure<RW::BSGeometryBounds>(data, dataI);

					/** VERTICES **/
					std::vector<glm::vec3> positions;
					positions.resize(geometry.numverts);
					for (size_t v = 0; v < geometry.numverts; ++v) {
						positions[v] = readStructure<glm::vec3>(data, dataI);
					}
					geom->setVertexData(positions);

					/** NORMALS **/
					if ((geometry.flags & RW::BSGeometry::StoreNormals) == RW::BSGeometry::StoreNormals) {
						std::vector<glm::vec3> normals;
						normals.reserve(geometry.numverts);
						for (size_t v = 0; v < geometry.numverts; ++v) {
							normals[v] = readStructure<glm::vec3>(data, dataI);
						}
						geom->setNormals(normals);
					}
					else {
						// Generate normals.
						/*geometryStruct.normals.resize(geometry.numverts);
						for (auto &subgeom : geometryStruct.subgeom) {
							glm::vec3 normal{0, 0, 0};
							for (size_t i = 0; i+2 < subgeom.indices.size(); i += 3) {
								glm::vec3 p1 = geometryStruct.vertices[subgeom.indices[i]];
								glm::vec3 p2 = geometryStruct.vertices[subgeom.indices[i+1]];
								glm::vec3 p3 = geometryStruct.vertices[subgeom.indices[i+2]];

								glm::vec3 U = p2 - p1;
								glm::vec3 V = p3 - p1;

								normal.x = (U.y * V.z) - (U.z * V.y);
								normal.y = (U.z * V.x) - (U.x * V.z);
								normal.z = (U.x * V.y) - (U.y * V.x);

								if (glm::length(normal) > 0.0000001)
									normal = glm::normalize(normal);

								geometryStruct.normals[subgeom.indices[i]]   = normal;
								geometryStruct.normals[subgeom.indices[i+1]] = normal;
								geometryStruct.normals[subgeom.indices[i+2]] = normal;
							}
						}*/
					}

					/** TEXTURES **/
					auto materiallistsec = item.getNextChildSection(secI);
					auto materialList = materiallistsec.readStructure<RW::BSMaterialList>();

					// Skip over the per-material byte values that I don't know what do.
					dataI += sizeof(uint32_t) * materialList.nummaterials;

					size_t matI = 0;
					materiallistsec.getNextChildSection(matI);
					
					geom->materials.resize(materialList.nummaterials);
					std::map<std::string, TextureInfo> availableTextures;

					for (size_t m = 0; m < materialList.nummaterials; ++m) {
						auto materialsec = materiallistsec.getNextChildSection(matI);
						if (materialsec.header.id != RW::SID_Material)
							continue;

						auto material = materialsec.readStructure<RW::BSMaterial>();
						geom->materials[m].textures.resize(material.numtextures);
						
						geom->materials[m].colour = material.color;
						geom->materials[m].diffuseIntensity = material.diffuse;
						geom->materials[m].ambientIntensity = material.ambient;

						size_t texI = 0;
						materialsec.getNextChildSection(texI);
						
						for (size_t t = 0; t < material.numtextures; ++t) {
							auto texsec = materialsec.getNextChildSection(texI);
							/*auto texture =*/ texsec.readStructure<RW::BSTexture>();

							std::string textureName, alphaName;
							size_t yetAnotherI = 0;
							texsec.getNextChildSection(yetAnotherI);

							auto namesec = texsec.getNextChildSection(yetAnotherI);
							auto alphasec = texsec.getNextChildSection(yetAnotherI);

							// The data is null terminated anyway.
							textureName = namesec.raw();
							alphaName = alphasec.raw();
							
							std::transform(textureName.begin(), textureName.end(), textureName.begin(), ::tolower );
							std::transform(alphaName.begin(), alphaName.end(), alphaName.begin(), ::tolower );

							geom->materials[m].textures[t] = {textureName, alphaName};
						}

						if(geom->materials[m].textures.size() < 1) continue;

						auto textureIt = gameData->textures
								.find(geom->materials[m].textures[0].name);
						if(textureIt != gameData->textures.end()) {
							availableTextures.insert({textureIt->first, textureIt->second});
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
								geom->subgeom.resize(meshplg.numsplits);
								geom->facetype = static_cast<Model::FaceType>(meshplg.facetype);
								size_t meshplgI = sizeof(RW::BSBinMeshPLG);
								for(size_t i = 0; i < meshplg.numsplits; ++i)
								{
                                    auto plgHeader = extsec.readSubStructure<RW::BSMaterialSplit>(meshplgI);
									meshplgI += sizeof(RW::BSMaterialSplit);
									geom->subgeom[i].material = plgHeader.index;
									geom->subgeom[i].indices = new uint32_t[plgHeader.numverts];
									geom->subgeom[i].numIndices = plgHeader.numverts;
									for (int j = 0; j < plgHeader.numverts; ++j) {
										geom->subgeom[i].indices[j] = extsec.readSubStructure<uint32_t>(meshplgI);
										meshplgI += sizeof(uint32_t);
									}
								}
							}
						}
					}

					geom->buildBuffers();
					
					geom->clumpNum = clumpID;
					
					// Add it
					model->geometries.push_back(geom);
				}
			}
			clumpID++;
			break;
		}
		case RW::SID_Atomic: {
			const Model::Atomic& at = sec.readStructure<Model::Atomic>();
			model->frames[at.frame]->addGeometry(at.geometry);
			model->atomics.push_back(at);
			break;
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
