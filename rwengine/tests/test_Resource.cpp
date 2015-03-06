#include <boost/test/unit_test.hpp>
#include <data/ResourceHandle.hpp>
#include <test_globals.hpp>

typedef ResourceHandle<int>::Ref IntRef;

BOOST_AUTO_TEST_SUITE(ResourceTests)

BOOST_AUTO_TEST_CASE(test_ResourceHandle)
{
	int resource = 42;
	IntRef ref { new ResourceHandle<int>("") };
	
	BOOST_CHECK_EQUAL( ref->resource, nullptr );
	BOOST_CHECK_EQUAL( ref->state, RW::Loading );
	
	ref->state = RW::Loaded;
	ref->resource = &resource;
	
	BOOST_CHECK_EQUAL( ref->resource, &resource );
	BOOST_CHECK_EQUAL( ref->state, RW::Loaded );
}

BOOST_AUTO_TEST_SUITE_END()