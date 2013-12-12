#include <boost/test/unit_test.hpp>
#include <renderwure/render/TextureAtlas.hpp>

/*BOOST_AUTO_TEST_SUITE(TextureAtlasTests)

BOOST_AUTO_TEST_CASE(atlas_fill_test)
{
	TextureAtlas atlas(16, 16);

	size_t dim = 16;

	BOOST_CHECK( atlas.canPack(&dim, &dim, 1) );

	float s, t, w, h;

	atlas.packTexture(nullptr, dim, dim, s, t, w, h);

	BOOST_CHECK( s == 0.f );
	BOOST_CHECK( t == 0.f );
	BOOST_CHECK( w == 1.f );
	BOOST_CHECK( h == 1.f );

	BOOST_CHECK( atlas.canPack(&dim, &dim, 1) == false );
}

BOOST_AUTO_TEST_CASE(atlas_pack_test)
{
	TextureAtlas atlas(4, 4);
	size_t dim = 1;

	uint8_t pixels[] = { 0xFF, 0x00, 0x00, 0xFF,
						 0x00, 0xFF, 0x00, 0xFF,
						 0x00, 0x00, 0xFF, 0xFF,
						 0x00, 0x00, 0x00, 0xFF };

	float s, t, w, h;

	atlas.packTexture(pixels+0, dim, dim, s, t, w, h);
	BOOST_CHECK( s == 0.f && t == 0.f && w == 0.25f && h == 0.25f );
	atlas.packTexture(pixels+4, dim, dim, s, t, w, h);
	BOOST_CHECK( s == 0.25f && t == 0.f && w == 0.25f && h == 0.25f );
	atlas.packTexture(pixels+8, dim, dim, s, t, w, h);
	BOOST_CHECK( s == 0.5f && t == 0.f && w == 0.25f && h == 0.25f );
	atlas.packTexture(pixels+12, dim, dim, s, t, w, h);
	BOOST_CHECK( s == 0.75f && t == 0.f && w == 0.25f && h == 0.25f );

	BOOST_CHECK( atlas.canPack(&dim, &dim, 1) == true );

	uint8_t outPixels[4*4*4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, outPixels);
	for(size_t p = 0; p < 16; ++p) {
		BOOST_CHECK(outPixels[p] == pixels[p]);
	}
}

BOOST_AUTO_TEST_SUITE_END()
*/