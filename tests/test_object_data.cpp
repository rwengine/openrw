#include <boost/test/unit_test.hpp>
#include <data/ObjectData.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(ObjectDataTests)

BOOST_AUTO_TEST_CASE(test_object_data)
{
	{
		LoaderIDE l;

		l.load(Global::get().getGamePath() + "/data/generic.ide");

		BOOST_ASSERT( l.objects.find(1100) != l.objects.end() );

		auto obj = l.objects[1100];

		std::shared_ptr<ObjectData> def = std::dynamic_pointer_cast<ObjectData>(def);

		BOOST_ASSERT(def->class_type == ObjectInformation::_class("OBJS"));

		BOOST_CHECK_EQUAL( def->modelName, "rd_corner1" );
		BOOST_CHECK_EQUAL( def->textureName, "generic" );
		BOOST_CHECK_EQUAL( def->numClumps, 1 );
		BOOST_CHECK_EQUAL( def->drawDistance[0], 220 );
		BOOST_CHECK_EQUAL( def->flags, 0 );
	}
	{
		LoaderIDE l;

		l.load(Global::get().getGamePath() + "/data/default.ide");

		BOOST_ASSERT( l.objects.find(90) != l.objects.end() );

		auto obj = l.objects[90];

		std::shared_ptr<VehicleData> def = std::dynamic_pointer_cast<VehicleData>(def);

		BOOST_ASSERT(def->type == ObjectInformation::_class("CARS"));

		BOOST_CHECK_EQUAL( def->modelName, "landstal");
		BOOST_CHECK_EQUAL( def->textureName, "landstal" );
		BOOST_CHECK_EQUAL( def->type, VehicleData::CAR );
		BOOST_CHECK_EQUAL( def->handlingID, "LANDSTAL" );
		BOOST_CHECK_EQUAL( def->gameName, "LANDSTK" );
		BOOST_CHECK_EQUAL( def->classType, VehicleData::RICHFAMILY );
		BOOST_CHECK_EQUAL( def->frequency, 7 );
		BOOST_CHECK_EQUAL( def->wheelModelID, 164 );
		BOOST_CHECK_EQUAL( def->wheelScale, 0.8 );
	}
}

BOOST_AUTO_TEST_SUITE_END()
