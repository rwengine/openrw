#include <renderwure/loaders/TextureLoader.hpp>
#include <renderwure/engine/GTAData.hpp>
#include <renderwure/render/TextureAtlas.hpp>

#include <fstream>
#include <iostream>

bool TextureLoader::loadFromFile(std::string filename, GTAData* gameData)
{
	std::ifstream dfile(filename);
	if ( ! dfile.is_open()) {
		std::cerr << "Error opening file " << filename << std::endl;
		return false;
	}

	dfile.seekg(0, std::ios_base::end);
	size_t length = dfile.tellg();
	dfile.seekg(0);
	char *data = new char[length];
	dfile.read(data, length);

	return loadFromMemory(data, gameData);
}

GLuint gErrorTextureData[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFF0000 };

bool TextureLoader::loadFromMemory(char *data, GTAData *gameData)
{
	RW::BinaryStreamSection root(data);
	auto texDict = root.readStructure<RW::BSTextureDictionary>();

	size_t rootI = 0;
	while (root.hasMoreData(rootI)) {
		auto rootSection = root.getNextChildSection(rootI);

		if (rootSection.header.id != RW::SID_TextureNative)
			continue;

		RW::BSTextureNative texNative = rootSection.readStructure<RW::BSTextureNative>();

		/// Short circuit for things we dont support.
		if(texNative.platform != 8) {
			std::cerr << "Unsupported texture platform " << std::dec << texNative.platform << std::endl;
			continue;
		}
		bool isPal4 = (texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL4) == RW::BSTextureNative::FORMAT_EXT_PAL4;
		bool isPal8 = (texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8) == RW::BSTextureNative::FORMAT_EXT_PAL8;
		bool isFulc = texNative.rasterformat == RW::BSTextureNative::FORMAT_1555 ||
					texNative.rasterformat == RW::BSTextureNative::FORMAT_8888 ||
					texNative.rasterformat == RW::BSTextureNative::FORMAT_888;
		if(! (isPal8 || isFulc)) {
			std::cerr << "Unsuported raster format " << std::dec << texNative.rasterformat << std::endl;
			continue;
		}

		bool useAtlas = false;

		GLuint textureName = 0;
		TextureAtlas* atlas = nullptr;
		glm::vec4 texRect(0.f, 0.f, 1.f, 1.f);
		if(useAtlas) {
			size_t ai = 0;
			size_t texW = texNative.width, texH = texNative.height;
			do {
				atlas = gameData->getAtlas(ai++);
			} while(! atlas->canPack(&texW, &texH, 1));
		}

		if(isPal8)
		{
			uint32_t fullColor[texNative.width * texNative.height];
			size_t paletteSize = 1024;
			bool hasAlpha = (texNative.rasterformat & RW::BSTextureNative::FORMAT_8888) == RW::BSTextureNative::FORMAT_8888;
			char* dataBase = rootSection.raw() + sizeof(RW::BSSectionHeader) + sizeof(RW::BSTextureNative);

			// Where does this -4 offset come from.
			uint8_t* coldata = reinterpret_cast<uint8_t*>(dataBase - 4 + paletteSize + sizeof(uint32_t));
			uint32_t raster_size = *reinterpret_cast<uint32_t*>(dataBase - 4 + paletteSize);
			uint32_t* palette = reinterpret_cast<uint32_t*>(dataBase - 4);

			uint32_t amask = hasAlpha ? 0xFF000000 : 0x0;
			for(size_t j = 0; j < raster_size; ++j)
			{
				fullColor[j] = palette[coldata[j]] | amask;
			}

			if(atlas) {
				atlas->packTextureFormat(
							fullColor, GL_BGRA, GL_UNSIGNED_BYTE,
							texNative.width, texNative.height,
							texRect.x, texRect.y, texRect.z, texRect.w);
			}
			else {
				glGenTextures(1, &textureName);
				glBindTexture(GL_TEXTURE_2D, textureName);
				glTexImage2D(
					GL_TEXTURE_2D, 0, GL_RGBA,
					texNative.width, texNative.height, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, fullColor
				);
			}
		}
		else if(isFulc)
		{
			auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative);
			coldata += sizeof(uint32_t);
			
			GLenum type, format;
			switch(texNative.rasterformat)
			{
				case RW::BSTextureNative::FORMAT_1555:
					format = GL_RGBA;
					type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
					break;
				case RW::BSTextureNative::FORMAT_8888:
					format = GL_RGBA;
					type = GL_UNSIGNED_INT_8_8_8_8_REV;
					break;
				case RW::BSTextureNative::FORMAT_888:
					format = GL_BGRA;
					type = GL_UNSIGNED_BYTE;
					break;
			}

			if(atlas) {
				atlas->packTextureFormat(
							coldata, format, type,
							texNative.width, texNative.height,
							texRect.x, texRect.y, texRect.z, texRect.w);
			}
			else {
				glGenTextures(1, &textureName);
				glBindTexture(GL_TEXTURE_2D, textureName);
				glTexImage2D(
					GL_TEXTURE_2D, 0, texNative.alpha ? GL_RGBA : GL_RGB,
					texNative.width, texNative.height, 0,
					format, type, coldata
				);
			}
		}

		std::string name = std::string(texNative.diffuseName);

		// todo: not completely ignore everything the TXD says.
		if(!atlas) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glGenerateMipmap(GL_TEXTURE_2D);
		}

		gameData->textures.insert({name, {textureName, atlas, texRect}});
	}

	return true;
}
