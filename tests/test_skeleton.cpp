#include <boost/test/unit_test.hpp>
#include <data/Skeleton.hpp>
#include <glm/glm.hpp>

BOOST_AUTO_TEST_SUITE(SkeletonTests)

BOOST_AUTO_TEST_CASE(test_methods) {
    Skeleton::FrameTransform t1{glm::vec3(0.f, 0.f, 0.f),
                                glm::quat(0.f, 0.f, 0.f, 0.f)};
    Skeleton::FrameTransform t2{glm::vec3(1.f, 0.f, 0.f),
                                glm::quat(0.f, 0.f, 1.f, 0.f)};

    Skeleton skeleton;

    skeleton.setAllData({{0, {t1, t2, true}}});

    BOOST_CHECK(skeleton.getData(0).a.translation == t1.translation);
    BOOST_CHECK(skeleton.getData(0).a.rotation == t1.rotation);

    BOOST_CHECK(skeleton.getData(0).b.translation == t2.translation);
    BOOST_CHECK(skeleton.getData(0).b.rotation == t2.rotation);

    BOOST_CHECK(skeleton.getData(0).enabled);

    skeleton.setData(0, {t2, t1, false});

    BOOST_CHECK(skeleton.getData(0).a.translation == t2.translation);
    BOOST_CHECK(skeleton.getData(0).a.rotation == t2.rotation);

    BOOST_CHECK(skeleton.getData(0).b.translation == t1.translation);
    BOOST_CHECK(skeleton.getData(0).b.rotation == t1.rotation);

    BOOST_CHECK(!skeleton.getData(0).enabled);
}

BOOST_AUTO_TEST_CASE(test_interpolate) {
    Skeleton::FrameTransform t1{glm::vec3(0.f, 0.f, 0.f),
                                glm::quat(0.f, 0.f, 0.f, 0.f)};
    Skeleton::FrameTransform t2{glm::vec3(1.f, 0.f, 0.f),
                                glm::quat(0.f, 0.f, 1.f, 0.f)};

    Skeleton skeleton;

    skeleton.setAllData({{0, {t2, t1, true}}});

    /** Without calling Skeleton::interpolate(alpha) the result is identity */

    BOOST_CHECK(skeleton.getInterpolated(0).translation ==
                Skeleton::IdentityTransform.translation);
    BOOST_CHECK(skeleton.getInterpolated(0).rotation ==
                Skeleton::IdentityTransform.rotation);

    skeleton.interpolate(0.f);

    BOOST_CHECK(skeleton.getInterpolated(0).translation == t1.translation);
    BOOST_CHECK(skeleton.getInterpolated(0).rotation == t1.rotation);

    skeleton.interpolate(1.f);

    BOOST_CHECK(skeleton.getInterpolated(0).translation == t2.translation);
    BOOST_CHECK(skeleton.getInterpolated(0).rotation == t2.rotation);
}
BOOST_AUTO_TEST_SUITE_END()
