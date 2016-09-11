#include <boost/test/unit_test.hpp>
#include <engine/GameData.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(GameDataTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_object_data) {
    GameData gd(&Global::get().log, &Global::get().work, Global::getGamePath());
    GameWorld gw(&Global::get().log, &Global::get().work, &gd);

    gd.load();

    {
        auto def = gd.findModelInfo<SimpleModelInfo>(1100);

        BOOST_REQUIRE(def);

        BOOST_ASSERT(def->type() == ModelDataType::SimpleInfo);

        BOOST_CHECK_EQUAL(def->name, "rd_Corner1");
        BOOST_CHECK_EQUAL(def->textureslot, "generic");
        BOOST_CHECK_EQUAL(def->getNumAtomics(), 1);
        BOOST_CHECK_EQUAL(def->getLodDistance(0), 220);
        BOOST_CHECK_EQUAL(def->flags, 0);
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
