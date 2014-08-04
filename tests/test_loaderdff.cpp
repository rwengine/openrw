#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <render/Model.hpp>
#include <WorkContext.hpp>

BOOST_AUTO_TEST_SUITE(LoaderDFFTests)

BOOST_AUTO_TEST_CASE(test_load_dff)
{
	{
		auto d = Global::get().e->gameData.openFile2("landstal.dff");

		LoaderDFF loader;

		Model* m = loader.loadFromMemory(d, &Global::get().e->gameData);

		BOOST_REQUIRE( m != nullptr );

		BOOST_REQUIRE_EQUAL( m->frames.size(), 40 );

		BOOST_REQUIRE_EQUAL( m->geometries.size(), 16 );

		BOOST_REQUIRE_EQUAL( m->geometries[0]->subgeom.size(), 5 );

		for(auto& g : m->geometries) {
			BOOST_CHECK_GT( g->geometryBounds.radius, 0.f );
		}

		BOOST_REQUIRE( m->atomics.size() > 0 );

		for(Model::Atomic& a : m->atomics) {
			BOOST_CHECK( a.frame < m->frames.size() );
			BOOST_CHECK( a.geometry < m->geometries.size() );
		}

		delete m;
	}

}

BOOST_AUTO_TEST_CASE(test_modeljob)
{
	{
		WorkContext ctx;

		Model* m = nullptr;
		bool done = false;
		LoadModelJob* lmj = new LoadModelJob(&ctx, &Global::get().e->gameData, "landstal.dff",
											 [&](Model* model) { m = model; done = true; });

		ctx.queueJob(lmj);

		while( ! done ) {
			ctx.update();
			std::this_thread::yield();
		}

		BOOST_REQUIRE( m != nullptr );

		BOOST_CHECK( m->frames.size() > 0 );

		delete m;
	}

}

BOOST_AUTO_TEST_SUITE_END()
