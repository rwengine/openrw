#include <boost/test/unit_test.hpp>
#include <data/Loader.hpp>
#include <test_globals.hpp>

class IntLoader : public Loader<int>
{
	int data;
public:
	IntLoader( int value ) : data( value ) { }
	
	ResultType get() { return data; }
};

BOOST_AUTO_TEST_SUITE(LoaderTests)

BOOST_AUTO_TEST_CASE(test_product)
{
	BOOST_CHECK( typeid(IntLoader::ResultType) == typeid(int) );
	IntLoader loader( 42 );
	BOOST_CHECK_EQUAL( loader.get(), 42 );
}

BOOST_AUTO_TEST_SUITE_END()