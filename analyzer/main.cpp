#include <renderware/BinaryStream.hpp>
#include <renderware/TextureArchive.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>
#include <sstream>

int main()
{
	std::cout << std::showbase;

	std::string gtapath = "/home/iostream/.wine/drive_c/Program Files (x86)/Rockstar Games/GTAIII/";
	auto ojg = RW::BinaryStream::parse(gtapath +"/models/MISC.TXD");
	// auto ojg = RW::BinaryStream::parse("OJG.TXD");

	auto texture = RW::TextureArchive::create(*ojg);

	std::cout << "Found " << texture->numTextures << " textures!" << std::endl;
	sf::Image img;
	for (size_t i = 0; i < texture->numTextures; i++) {
		auto &tex = texture->textures[i];
		std::cout << "Processing " << tex.header.diffuseName << std::endl;

		img.create(tex.header.width, tex.header.height);

		for (int j = 0; j < tex.header.width; j++) {
			for (int i = 0; i < tex.header.width; i++) {
				bool hasAlpha = (tex.header.rasterFormat & 0x0500) == 0x0500;
				if (tex.header.rasterFormat & 0x2000) {
					uint8_t pixelIndex = 4 * tex.body.pixels[j*tex.header.width + i];

					img.setPixel(i, j, {
						tex.body.palette[pixelIndex + 0],
						tex.body.palette[pixelIndex + 1],
						tex.body.palette[pixelIndex + 2],
						(hasAlpha ? tex.body.palette[pixelIndex + 3] : static_cast<sf::Uint8>(255))
					});
				} else {
					uint8_t pixel = 4 * tex.body.pixels[j*tex.header.width + i];
					img.setPixel(i, j, {
						tex.body.pixels[pixel + 0],
						tex.body.pixels[pixel + 1],
						tex.body.pixels[pixel + 2],
						tex.body.pixels[pixel + 3],
					});
				}
			}
		}

		std::stringstream ss;
		ss << tex.header.diffuseName << ".png";
		img.saveToFile(ss.str());

		size_t scaleUp = 8;
		img.create(16*scaleUp, 16*scaleUp);
		for (int j = 0; j < 16; j++) {
			for (int i = 0; i < 16; i++) {
				uint8_t pixelIndex = 4 * (j*16 + i);

				for (int y = 0; y < scaleUp; y++) {
					for (int x = 0; x < scaleUp; x++) {
						img.setPixel((i*scaleUp)+x, (j*scaleUp)+y, {
							tex.body.palette[pixelIndex + 0],
							tex.body.palette[pixelIndex + 1],
							tex.body.palette[pixelIndex + 2],
							tex.body.palette[pixelIndex + 3],
						});
					}
				}
			}
		}
		img.saveToFile("palette"+ ss.str());
	}

	return 0;
}
