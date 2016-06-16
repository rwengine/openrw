#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <data/Model.hpp>
#include <job/WorkContext.hpp>
#include <loaders/BackgroundLoader.hpp>

BOOST_AUTO_TEST_SUITE(LoaderDFFTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_dff)
{
	{
		auto d = Global::get().e->data->openFile("landstal.dff");

		LoaderDFF loader;

		Model* m = loader.loadFromMemory(d);

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

BOOST_AUTO_TEST_CASE(test_loader_job)
{
	{
		WorkContext ctx;

		ResourceHandle<Model>::Ref modelRef { new ResourceHandle<Model>("landstal.dff") };
		
		auto index = &Global::get().e->data->index;
		auto job = new BackgroundLoaderJob<Model, LoaderDFF>{ &ctx, index, "landstal.dff", modelRef };

		ctx.queueJob(job);

		while( modelRef->state == RW::Loading ) {
			ctx.update();
			std::this_thread::yield();
		}

		BOOST_REQUIRE( modelRef->resource != nullptr );

		BOOST_CHECK( modelRef->resource->frames.size() > 0 );
		
		delete modelRef->resource;
	}

}
#endif

BOOST_AUTO_TEST_SUITE_END()
