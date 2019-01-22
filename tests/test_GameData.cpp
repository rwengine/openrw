#include <boost/test/unit_test.hpp>
#include <engine/GameData.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(GameDataTests, DATA_TEST_PREDICATE)

BOOST_AUTO_TEST_CASE(test_object_data) {
    GameData gd(&Global::get().log, Global::getGamePath());
    gd.load();

    GameWorld gw(&Global::get().log, &gd);
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

BOOST_AUTO_TEST_CASE(test_ped_relations) {
    GameData gd(&Global::get().log, Global::getGamePath());
    gd.load();

    auto& rel_cop = gd.pedrels[PedModelInfo::COP];
    auto& rel_crim = gd.pedrels[PedModelInfo::CRIMINAL];

    BOOST_CHECK_EQUAL(rel_cop.id_, PedRelationship::THREAT_COP);
    BOOST_CHECK_EQUAL(rel_cop.threatflags_,
                      PedRelationship::THREAT_GUN |
                          PedRelationship::THREAT_EXPLOSION |
                          PedRelationship::THREAT_DEADPEDS);
    BOOST_CHECK_EQUAL(rel_crim.id_, PedRelationship::THREAT_CRIMINAL);
    BOOST_CHECK_EQUAL(rel_crim.threatflags_,
                      PedRelationship::THREAT_GUN |
                          PedRelationship::THREAT_COP |
                          PedRelationship::THREAT_COP_CAR |
                          PedRelationship::THREAT_EXPLOSION);
}

BOOST_AUTO_TEST_CASE(test_ped_groups) {
    GameData gd(&Global::get().log, Global::getGamePath());
    gd.load();

    BOOST_REQUIRE(gd.pedgroups.size() > 2);

    const auto& def = gd.pedgroups[0];
    const auto& red = gd.pedgroups[1];

    BOOST_REQUIRE_GE(def.size(), 8);
    BOOST_CHECK_EQUAL(def[0], 30);

    BOOST_REQUIRE_GE(red.size(), 8);
    BOOST_CHECK_EQUAL(red[0], 34);
}

BOOST_AUTO_TEST_SUITE_END()
