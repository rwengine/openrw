#include <boost/test/unit_test.hpp>
#include <loaders/LoadContext.hpp>
#include <loaders/DataLoader.hpp>



class TestLoader : public DataLoader
{
public:
	bool _loaded, _loadResult, _created;

	TestLoader(LoadContext* context, bool result )
		: DataLoader(context), _loaded(false), _loadResult(result),
		_created(false) { }

	bool load( const char* bytes, size_t size )
	{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); _loaded = true; return _loadResult; }

	void create() { _created = true; }
};


BOOST_AUTO_TEST_SUITE(LoaderTests)


BOOST_AUTO_TEST_CASE(test_interface)
{
	{
		LoadContext context;

		TestLoader loader( &context, true );

		BOOST_CHECK_EQUAL( loader.getContext(), &context );

		// Test that the TestLoader works as expected.
		const char* databytes = new const char[1]{ 0x00 };

		BOOST_CHECK( ! loader._loaded );
		BOOST_CHECK( ! loader._created );

		BOOST_CHECK( loader.load(databytes, 1) );

		BOOST_CHECK( loader._loaded );
		BOOST_CHECK( ! loader._created );

		loader.create();

		BOOST_CHECK( loader._created );
	}
}

BOOST_AUTO_TEST_CASE(test_context)
{
	{
		LoadContext context;

		TestLoader test(&context, true);
		const char* databytes = new const char[1]{ 0x00 };

		context.add(&test, databytes, 1);

		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );
		BOOST_CHECK_EQUAL( context.getTotal(), 1 );
		BOOST_CHECK_EQUAL( context.getComplete(), 0 );

		auto& queue = context.getLoadQueue();
		BOOST_REQUIRE( queue.size() == 1 );
		BOOST_CHECK_EQUAL( queue.front().loader, &test );
		BOOST_CHECK_EQUAL( queue.front().data, databytes );
		BOOST_CHECK_EQUAL( queue.front().size, 1 );

		// Worker thread tier.
		context.loadNext();

		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );
		BOOST_CHECK_EQUAL( context.getTotal(), 1 );
		BOOST_CHECK_EQUAL( context.getComplete(), 0 );

		// Back on main thread
		BOOST_CHECK_EQUAL( context.getLoaded(), 1 );

		context.flushCreation();

		BOOST_CHECK_EQUAL( context.getIncomplete(), 0 );
		BOOST_CHECK_EQUAL( context.getTotal(), 1 );
		BOOST_CHECK_EQUAL( context.getComplete(), 1 );

	}
}

BOOST_AUTO_TEST_CASE(test_thread)
{
	{
		LoadContext context;

		LoadWorker worker(&context);

		TestLoader test(&context, true);
		const char* databytes = new const char[1]{ 0x00 };

		context.add(&test, databytes, 1);

		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );
		BOOST_CHECK_EQUAL( context.getLoaded(), 0 );

		worker._started = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		BOOST_CHECK_EQUAL( context.getLoaded(), 1 );
		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );

		context.flushCreation();

		BOOST_CHECK_EQUAL( context.getLoaded(), 0 );
		BOOST_CHECK_EQUAL( context.getIncomplete(), 0 );
		BOOST_CHECK_EQUAL( context.getComplete(), 1 );
	}
	{
		LoadContext context;

		LoadWorker worker(&context);

		TestLoader test(&context, true);
		const char* databytes = new const char[1]{ 0x00 };

		context.add(&test, databytes, 1);

		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );
		BOOST_CHECK_EQUAL( context.getLoaded(), 0 );

		worker._started = true;

		context.wait();

		BOOST_CHECK_EQUAL( context.getLoaded(), 1 );
		BOOST_CHECK_EQUAL( context.getIncomplete(), 1 );

		context.flushCreation();

		BOOST_CHECK_EQUAL( context.getLoaded(), 0 );
		BOOST_CHECK_EQUAL( context.getIncomplete(), 0 );
		BOOST_CHECK_EQUAL( context.getComplete(), 1 );
	}
}
BOOST_AUTO_TEST_SUITE_END()

