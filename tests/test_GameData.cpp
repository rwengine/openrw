#include <boost/test/unit_test.hpp>
#include <engine/GameData.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(GameDataTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_object_data) {
    GameData gd(&Global::get().log, Global::getGamePath());
    GameWorld gw(&Global::get().log, &gd);

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

BOOST_AUTO_TEST_CASE(test_ped_stats) {
    GameData gd(&Global::get().log, Global::getGamePath());
    gd.load();

    BOOST_REQUIRE(gd.pedstats.size() > 2);

    auto& stat1 = gd.pedstats[0];
    auto& stat2 = gd.pedstats[1];

    BOOST_CHECK_EQUAL(stat1.id_, 0);
    BOOST_CHECK_EQUAL(stat1.fleedistance_, 0.0f);
    BOOST_CHECK_EQUAL(stat1.defendweakness_, 0.4f);
    BOOST_CHECK_EQUAL(stat1.flags_, 6);

    BOOST_CHECK_EQUAL(stat2.id_, 1);
    BOOST_CHECK_EQUAL(stat2.fleedistance_, 20.0f);
    BOOST_CHECK_EQUAL(stat2.defendweakness_, 1.0f);
    BOOST_CHECK_EQUAL(stat2.flags_, 2);
}

BOOST_AUTO_TEST_CASE(test_ped_stat_info) {
    GameData gd(&Global::get().log, Global::getGamePath());
    gd.load();

    BOOST_REQUIRE(gd.pedstats.size() > 2);
    BOOST_REQUIRE(gd.modelinfo.find(1) != gd.modelinfo.end());
    auto it = gd.modelinfo.find(1);
    auto cop = static_cast<PedModelInfo*>(it->second.get());

    auto& stat_cop = gd.pedstats[1];

    BOOST_CHECK_EQUAL(cop->statindex_, stat_cop.id_);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
