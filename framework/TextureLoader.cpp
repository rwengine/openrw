#include "TextureLoader.h"

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
		
		if(texNative.platform != 8) 
		{
			std::cerr << "Unsupported texture platform " << std::dec << texNative.platform << std::endl;
			continue;
		}
		
		GLuint texture = 0;
		
		if((texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8) == RW::BSTextureNative::FORMAT_EXT_PAL8)
		{
			auto palette = rootSection.readSubStructure<RW::BSPaletteData>(sizeof(RW::BSTextureNative));
			auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative) + sizeof(RW::BSPaletteData);

			uint8_t fullColor[texNative.width * texNative.height * 4];

			for (size_t y = 0; y < texNative.height; y++) {
				for (size_t x = 0; x < texNative.width; x++) {
					size_t texI = ((y * texNative.width) + x) * 4;
					size_t palI = 4 * static_cast<size_t>(
						coldata[(y * texNative.width) + x]
					);

					fullColor[texI+0] = palette.palette[palI+2];
					fullColor[texI+1] = palette.palette[palI+1];
					fullColor[texI+2] = palette.palette[palI+0];
					fullColor[texI+3] = palette.palette[palI+3];
				}
			}
			
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				texNative.width, texNative.height, 0,
				GL_BGRA, GL_UNSIGNED_BYTE, fullColor
			);
		}
		else if(
			texNative.rasterformat == RW::BSTextureNative::FORMAT_1555 ||
			texNative.rasterformat == RW::BSTextureNative::FORMAT_8888 ||
			texNative.rasterformat == RW::BSTextureNative::FORMAT_888
		)
		{
			auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative) + sizeof(uint32_t);
			
			GLenum type, format;
			switch(texNative.rasterformat)
			{
				case RW::BSTextureNative::FORMAT_1555:
					format = GL_RGBA;
					type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
					break;
				case RW::BSTextureNative::FORMAT_8888:
					format = GL_BGRA;
					type = GL_UNSIGNED_BYTE;
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
