#include <renderwure/loaders/TextureLoader.hpp>

#include <fstream>
#include <iostream>

bool TextureLoader::loadFromFile(std::string filename)
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

	return loadFromMemory(data);
}

GLuint gErrorTextureData[] = { 0x00FF00FF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF00FF };

bool TextureLoader::loadFromMemory(char *data)
{
	RW::BinaryStreamSection root(data);
	auto texDict = root.readStructure<RW::BSTextureDictionary>();

	size_t rootI = 0;
	while (root.hasMoreData(rootI)) {
		auto rootSection = root.getNextChildSection(rootI);

		if (rootSection.header.id != RW::SID_TextureNative)
			continue;

		auto texNative = rootSection.readStructure<RW::BSTextureNative>();
		
		GLuint texture = 0;
		
		if(texNative.platform != 8) 
		{
			std::cerr << "Unsupported texture platform " << std::dec << texNative.platform << std::endl;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				2, 2, 0,
				GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, gErrorTextureData
			);
		}
		else if((texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8) == RW::BSTextureNative::FORMAT_EXT_PAL8)
		{
			auto palette = rootSection.readSubStructure<RW::BSPaletteData>(sizeof(RW::BSSectionHeader)+sizeof(RW::BSTextureNative) - 4);
			uint8_t* coldata = reinterpret_cast<uint8_t*>(rootSection.raw() + sizeof(RW::BSSectionHeader) + sizeof(RW::BSTextureNative) + sizeof(RW::BSPaletteData) - 4);
			uint8_t fullColor[texNative.width * texNative.height * 4];
			
			bool hasAlpha = texNative.alpha == 1;
			
			for(size_t j = 0; j < texNative.width * texNative.height; ++j)
			{
				size_t iTex = j * 4;
				size_t iPal = coldata[j] * 4;
				fullColor[iTex+0] = palette.palette[iPal+0];
				fullColor[iTex+1] = palette.palette[iPal+1];
				fullColor[iTex+2] = palette.palette[iPal+2];
				fullColor[iTex+3] = hasAlpha ? palette.palette[iPal+3] : 255;
			}
			
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				texNative.width, texNative.height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, fullColor
			);
		}
		else if(
			texNative.rasterformat == RW::BSTextureNative::FORMAT_1555 ||
			texNative.rasterformat == RW::BSTextureNative::FORMAT_8888 ||
			texNative.rasterformat == RW::BSTextureNative::FORMAT_888
		)
		{
			auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative);
			uint32_t rastersize = *coldata;
			coldata += sizeof(uint32_t);
			
			GLenum type, format;
			switch(texNative.rasterformat)
			{
				case RW::BSTextureNative::FORMAT_1555:
					format = GL_RGBA;
					type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
					break;
				case RW::BSTextureNative::FORMAT_8888:
					format = GL_BGRA;
					type = GL_UNSIGNED_INT_8_8_8_8_REV;
					break;
				case RW::BSTextureNative::FORMAT_888:
					format = GL_BGR;
					type = GL_UNSIGNED_BYTE;
					break;
			}
			
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				texNative.width, texNative.height, 0,
				format, type, coldata
			);
		}
		else 
		{
			std::cerr << "Unsuported raster format " << std::hex << texNative.rasterformat << std::endl;
		}
		
		if(texture != 0) 
		{
			// todo: not completely ignore everything the TXD says.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glGenerateMipmap(GL_TEXTURE_2D);

			std::string name = std::string(texNative.diffuseName);

			textures[name] = texture;
		}

		// std::cout << "Loaded texture '" << name << "'" << std::endl;
	}

	return true;
}

void TextureLoader::bindTexture(std::string texture)
{
	if (textures.find(texture) == textures.end()) {
		// std::cerr << "Could not find nor bind texture '" << texture << "'" << std::endl;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, textures[texture]);
}
