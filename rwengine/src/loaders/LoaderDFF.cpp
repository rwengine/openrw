#include <loaders/LoaderDFF.hpp>
#include <engine/GameData.hpp>
#include <render/Model.hpp>

#include <iostream>
#include <algorithm>
#include <numeric>
#include <set>
#include <cstring>

enum DFFChunks
{
	CHUNK_STRUCT       = 0x0001,
	CHUNK_EXTENSION    = 0x0003,
	CHUNK_TEXTURE      = 0x0006,
	CHUNK_MATERIAL     = 0x0007,
	CHUNK_MATERIALLIST = 0x0008,
	CHUNK_FRAMELIST    = 0x000E,
	CHUNK_GEOMETRY     = 0x000F,
	CHUNK_CLUMP        = 0x0010,

	CHUNK_ATOMIC       = 0x0014,

	CHUNK_GEOMETRYLIST = 0x001A,

	CHUNK_BINMESHPLG   = 0x050E,

	CHUNK_NODENAME = 0x0253F2FE,
};

// These structs are used to interpret raw bytes from the stream.
/// @todo worry about endianness.

typedef glm::vec3 BSTVector3;
typedef glm::mat3 BSTMatrix;
typedef glm::i8vec4 BSTColour;

struct RWBSFrame
{
	BSTMatrix rotation;
	BSTVector3 position;
	int32_t index;
	uint32_t matrixflags; // Not used
};

void LoaderDFF::readFrameList(Model *model, const RWBStream &stream)
{
	auto listStream = stream.getInnerStream();

	auto listStructID = listStream.getNextChunk();
	if( listStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Frame List missing struct chunk");
	}

	char* headerPtr = listStream.getCursor();

	unsigned int numFrames = *(std::uint32_t*)headerPtr;
	headerPtr += sizeof(std::uint32_t);

	model->frames.reserve(numFrames);

	for( size_t f = 0; f < numFrames; ++f ) {
		auto data = (RWBSFrame*)headerPtr;
		headerPtr += sizeof(RWBSFrame);

		ModelFrame* parent = nullptr;
		if( data->index != -1 ) {
			parent = model->frames[data->index];
		}
		else {
			model->rootFrameIdx = f;
		}

		auto frame = new ModelFrame(parent, data->rotation, data->position);
		model->frames.push_back(frame);
	}

	size_t namedFrames = 0;

	/// @todo perhaps flatten this out a little
	for( auto chunkID = listStream.getNextChunk(); chunkID != 0; chunkID = listStream.getNextChunk() )
	{
		switch(chunkID) {
		case CHUNK_EXTENSION: {
			auto extStream = listStream.getInnerStream();
			for( auto chunkID = extStream.getNextChunk(); chunkID != 0; chunkID = extStream.getNextChunk() )
			{
				switch( chunkID ) {
				case CHUNK_NODENAME: {
					std::string fname(extStream.getCursor(), extStream.getCurrentChunkSize());
					std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower );

					if( namedFrames < model->frames.size() ) {
						model->frames[namedFrames++]->setName(fname);
					}
				}
					break;
				default:
					break;
				}
			}
		}
			break;
		default:
			break;
		}
	}
}

void LoaderDFF::readGeometryList(Model *model, const RWBStream &stream)
{
	auto listStream = stream.getInnerStream();

	auto listStructID = listStream.getNextChunk();
	if( listStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Geometry List missing struct chunk");
	}

	char* headerPtr = listStream.getCursor();

	unsigned int numGeometries = *(std::uint32_t*)headerPtr;
	headerPtr += sizeof(std::uint32_t);

	model->geometries.reserve(numGeometries);

	for( auto chunkID = listStream.getNextChunk(); chunkID != 0; chunkID = listStream.getNextChunk() )
	{
		switch(chunkID) {
		case CHUNK_GEOMETRY:
			readGeometry(model, listStream);
			break;
		default:
			break;
		}
	}
}

void LoaderDFF::readGeometry(Model *model, const RWBStream &stream)
{
	auto geomStream = stream.getInnerStream();

	auto geomStructID = geomStream.getNextChunk();
	if( geomStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Geometry missing struct chunk");
	}

	std::shared_ptr<Model::Geometry> geom(new Model::Geometry);

	char* headerPtr = geomStream.getCursor();

	geom->flags = *(std::uint16_t*)headerPtr;
	headerPtr += sizeof(std::uint16_t);

	/*unsigned short numUVs = *(std::uint8_t*)headerPtr;*/
	headerPtr += sizeof(std::uint8_t);
	/*unsigned short moreFlags = *(std::uint8_t*)headerPtr;*/
	headerPtr += sizeof(std::uint8_t);

	unsigned int numTris = *(std::uint32_t*)headerPtr;
	headerPtr += sizeof(std::uint32_t);
	unsigned int numVerts = *(std::uint32_t*)headerPtr;
	headerPtr += sizeof(std::uint32_t);
	/*unsigned int numFrames = *(std::uint32_t*)headerPtr;*/
	headerPtr += sizeof(std::uint32_t);

	std::vector<Model::GeometryVertex> verts;
	verts.resize(numVerts);

	if( geomStream.getChunkVersion() < 0x1003FFFF ) {
		headerPtr += sizeof(RW::BSGeometryColor);
	}

	/// @todo extract magic numbers.

	if( (geom->flags & 8) == 8 ) {
		for(size_t v = 0; v < numVerts; ++v) {
			verts[v].colour = *(glm::u8vec4*)headerPtr;
			headerPtr += sizeof(glm::u8vec4);
		}
	}
	else {
		for(size_t v = 0; v < numVerts; ++v) {
			verts[v].colour = {255, 255, 255, 255};
		}
	}

	if( (geom->flags & 4) == 4 || (geom->flags & 128) == 128) {
		for(size_t v = 0; v < numVerts; ++v) {
			verts[v].texcoord = *(glm::vec2*)headerPtr;
			headerPtr += sizeof(glm::vec2);
		}
	}

	// Skip indicies data for now.
	headerPtr += sizeof(RW::BSGeometryTriangle) * numTris;

	geom->geometryBounds = *(RW::BSGeometryBounds*)headerPtr;
	geom->geometryBounds.radius = std::abs(geom->geometryBounds.radius);
	headerPtr += sizeof(RW::BSGeometryBounds);

	for(size_t v = 0; v < numVerts; ++v) {
		verts[v].position = *(glm::vec3*)headerPtr;
		headerPtr += sizeof(glm::vec3);
	}

	if( (geom->flags & 16) == 16 ) {
		for(size_t v = 0; v < numVerts; ++v) {
			verts[v].normal = *(glm::vec3*)headerPtr;
			headerPtr += sizeof(glm::vec3);
		}
	}

	// Add the geometry to the model now so that it can be accessed.
	model->geometries.push_back(geom);

	// Process the geometry child sections
	for(auto chunkID = geomStream.getNextChunk(); chunkID != 0; chunkID = geomStream.getNextChunk())
	{
		switch( chunkID ) {
		case CHUNK_MATERIALLIST:
			readMaterialList(model, geomStream);
			break;
		case CHUNK_EXTENSION:
			readGeometryExtension(model, geomStream);
			break;
		default:
			break;
		}
	}

	geom->dbuff.setFaceType(geom->facetype == Model::Triangles ?
									GL_TRIANGLES : GL_TRIANGLE_STRIP);
	geom->gbuff.uploadVertices(verts);
	geom->dbuff.addGeometry(&geom->gbuff);

	glGenBuffers(1, &geom->EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->EBO);

	size_t icount = std::accumulate(geom->subgeom.begin(), geom->subgeom.end(),
									0u,
									[](size_t a, const Model::SubGeometry& b) {return a + b.numIndices;});
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * icount, 0, GL_STATIC_DRAW);
	for(auto& sg : geom->subgeom) {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
						sg.start * sizeof(uint32_t),
						sizeof(uint32_t) * sg.numIndices,
						sg.indices.data());
	}

}

void LoaderDFF::readMaterialList(Model *model, const RWBStream &stream)
{
	auto listStream = stream.getInnerStream();

	auto listStructID = listStream.getNextChunk();
	if( listStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("MaterialList missing struct chunk");
	}

	unsigned int numMaterials = *(std::uint32_t*)listStream.getCursor();

	model->geometries.back()->materials.reserve(numMaterials);

	RWBStream::ChunkID chunkID;
	while( (chunkID = listStream.getNextChunk()) ) {
		switch( chunkID ) {
		case CHUNK_MATERIAL:
			readMaterial(model, listStream);
			break;
		default:
			break;
		}
	}
}

void LoaderDFF::readMaterial(Model *model, const RWBStream &stream)
{
	auto materialStream = stream.getInnerStream();

	auto matStructID = materialStream.getNextChunk();
	if( matStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Material missing struct chunk");
	}

	char* matData = materialStream.getCursor();

	Model::Material material;

	// Unkown
	matData += sizeof(std::uint32_t);
	material.colour = *(glm::u8vec4*)matData;
	matData += sizeof(std::uint32_t);
	// Unkown
	matData += sizeof(std::uint32_t);
	/*bool usesTexture = *(std::uint32_t*)matData;*/
	matData += sizeof(std::uint32_t);

	material.ambientIntensity = *(float*)matData;
	matData += sizeof(float);
	/*float specular = *(float*)matData;*/
	matData += sizeof(float);
	material.diffuseIntensity = *(float*)matData;
	matData += sizeof(float);
	material.flags = 0;

	model->geometries.back()->materials.push_back(material);

	RWBStream::ChunkID chunkID;
	while( ( chunkID = materialStream.getNextChunk() ) ) {
		switch( chunkID ) {
		case CHUNK_TEXTURE:
			readTexture(model, materialStream);
			break;
		default:
			break;
		}
	}
}

void LoaderDFF::readTexture(Model *model, const RWBStream &stream)
{
	auto texStream = stream.getInnerStream();

	auto texStructID = texStream.getNextChunk();
	if( texStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Texture missing struct chunk");
	}

	// There's some data in the Texture's struct, but we don't know what it is.

	/// @todo improve how these strings are read.
	std::string name, alpha;

	texStream.getNextChunk();
	name = texStream.getCursor();
	texStream.getNextChunk();
	alpha = texStream.getCursor();

	std::transform(name.begin(), name.end(), name.begin(), ::tolower );
	std::transform(alpha.begin(), alpha.end(), alpha.begin(), ::tolower );

	model->geometries.back()->materials.back().textures.push_back({name, alpha});
}

void LoaderDFF::readGeometryExtension(Model *model, const RWBStream &stream)
{
	auto extStream = stream.getInnerStream();

	RWBStream::ChunkID chunkID;
	while( (chunkID = extStream.getNextChunk()) ) {
		switch( chunkID ) {
		case CHUNK_BINMESHPLG:
			readBinMeshPLG(model, extStream);
			break;
		default:
			break;
		}
	}
}

void LoaderDFF::readBinMeshPLG(Model *model, const RWBStream &stream)
{
	auto data = stream.getCursor();

	model->geometries.back()->facetype = static_cast<Model::FaceType>(
				*(std::uint32_t*)data);
	data += sizeof(std::uint32_t);

	unsigned int numSplits = *(std::uint32_t*)data;
	data += sizeof(std::uint32_t);

	// Number of triangles.
	data += sizeof(std::uint32_t);

	model->geometries.back()->subgeom.reserve(numSplits);

	size_t start = 0;

	for(size_t s = 0; s < numSplits; ++s) {
		Model::SubGeometry sg;
		sg.numIndices = *(std::uint32_t*)data;
		data += sizeof(std::uint32_t);
		sg.material = *(std::uint32_t*)data;
		data += sizeof(std::uint32_t);
		sg.start = start;
		start += sg.numIndices;

		sg.indices.resize(sg.numIndices);
		std::memcpy(sg.indices.data(), data, sizeof(std::uint32_t) * sg.numIndices);
		data += sizeof(std::uint32_t) * sg.numIndices;

		model->geometries.back()->subgeom.push_back(sg);
	}
}

void LoaderDFF::readAtomic(Model *model, const RWBStream &stream)
{
	auto atomicStream = stream.getInnerStream();

	auto atomicStructID = atomicStream.getNextChunk();
	if( atomicStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Atomic missing struct chunk");
	}

	Model::Atomic atom;
	auto data = atomicStream.getCursor();
	atom.frame = *(std::uint32_t*)data;
	data += sizeof(std::uint32_t);
	atom.geometry = *(std::uint32_t*)data;
	model->frames[atom.frame]->addGeometry(atom.geometry);
	model->atomics.push_back(atom);

	/// @todo are any atomic extensions important?
}

Model* LoaderDFF::loadFromMemory(FileHandle file, GameData *gameData)
{
	auto model = new Model;

	RWBStream rootStream(file->data, file->length);

	auto rootID = rootStream.getNextChunk();
	if( rootID != CHUNK_CLUMP ) {
		throw DFFLoaderException("Invalid root section ID " + std::to_string(rootID));
	}

	RWBStream modelStream = rootStream.getInnerStream();
	auto rootStructID = modelStream.getNextChunk();
	if( rootStructID != CHUNK_STRUCT ) {
		throw DFFLoaderException("Clump missing struct chunk");
	}

	// There is only one value in the struct section.
	model->numAtomics = *(std::uint32_t*)rootStream.getCursor();

	// Process everything inside the clump stream.
	RWBStream::ChunkID chunkID;
	while( ( chunkID = modelStream.getNextChunk() ) ) {
		switch( chunkID ) {
		case CHUNK_FRAMELIST:
			readFrameList(model, modelStream);
			break;
		case CHUNK_GEOMETRYLIST:
			readGeometryList(model, modelStream);
			break;
		case CHUNK_ATOMIC:
			readAtomic(model, modelStream);
			break;
		default:
			break;
		}
	}

	return model;
}

LoadModelJob::LoadModelJob(WorkContext *context, GameData* gd, const std::string &file, ModelCallback cb)
	: WorkJob(context), _gameData(gd), _file(file), _callback(cb), _data(nullptr)
{

}

void LoadModelJob::work()
{
	_data = _gameData->openFile2(_file);
}

void LoadModelJob::complete()
{
	Model* m = nullptr;
	// TODO error status
	if( _data ) {

		// TODO allow some of the loading to process in a seperate thread.
		LoaderDFF loader;

		m = loader.loadFromMemory(_data, _gameData);
	}

	_callback(m);
}
