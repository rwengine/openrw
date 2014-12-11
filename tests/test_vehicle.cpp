#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/VehicleObject.hpp>
#include <render/Model.hpp>
#include <data/Skeleton.hpp>

BOOST_AUTO_TEST_SUITE(VehicleTests)

BOOST_AUTO_TEST_CASE(test_create_vehicle)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	// Hardcoded values for the moment
	BOOST_CHECK_EQUAL(vehicle->vehicle->type, VehicleData::CAR);

	BOOST_CHECK_EQUAL(vehicle->info->wheels.size(), 4);

	BOOST_CHECK_EQUAL(vehicle->info->seats.size(), 4);

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(vehicle_frame_vis)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);
	BOOST_REQUIRE(vehicle->model != nullptr);

	auto bonnet_ok = vehicle->model->model->findFrame("bonnet_hi_ok");
	auto bonnet_dam = vehicle->model->model->findFrame("bonnet_hi_dam");

	BOOST_REQUIRE(bonnet_ok != nullptr);
	BOOST_REQUIRE(bonnet_dam != nullptr);

	BOOST_CHECK( vehicle->skeleton->getData(bonnet_ok->getIndex()).enabled );
	BOOST_CHECK(!vehicle->skeleton->getData(bonnet_dam->getIndex()).enabled);

	vehicle->setFrameState(bonnet_ok, VehicleObject::DAM);

	BOOST_CHECK(!vehicle->skeleton->getData(bonnet_ok->getIndex()).enabled );
	BOOST_CHECK( vehicle->skeleton->getData(bonnet_dam->getIndex()).enabled);

	vehicle->setFrameState(bonnet_ok, VehicleObject::OK);

	BOOST_CHECK( vehicle->skeleton->getData(bonnet_ok->getIndex()).enabled );
	BOOST_CHECK(!vehicle->skeleton->getData(bonnet_dam->getIndex()).enabled);

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_door_position)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	BOOST_CHECK( vehicle->getSeatEntryPosition(0).x > 5.f );


	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_hinges)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());

	BOOST_CHECK( vehicle->_hingedObjects.size() > 0 );

	for(auto& ho : vehicle->_hingedObjects) {
		// All Hinge objects should initalize, but not be locked.
		BOOST_CHECK( ho.second.body == nullptr );
	}

	auto fld = vehicle->model->model->findFrame("door_lf_dummy");

	vehicle->setHingeLocked(fld, false);

	BOOST_REQUIRE( vehicle->_hingedObjects[fld].body != nullptr );

	vehicle->setHingeLocked(fld, true);

	BOOST_CHECK( vehicle->_hingedObjects[fld].body == nullptr );
}

BOOST_AUTO_TEST_SUITE_END()

