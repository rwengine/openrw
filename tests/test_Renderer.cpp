#include <boost/test/unit_test.hpp>
#include <render/LegacyGameRenderer.hpp>

BOOST_AUTO_TEST_SUITE(RendererTests)

BOOST_AUTO_TEST_CASE(frustum_test_visible) {
    {
        ViewFrustum f(0.1f, 100.f, glm::half_pi<float>(), 1.f);

        f.update(f.projection());

        BOOST_CHECK(f.intersects({10.f, 0.f, -10.f}, 1.f));
        BOOST_CHECK(!f.intersects({0.f, 0.f, 10.f}, 1.f));

        BOOST_CHECK(!f.intersects({0.f, 10.f, 0.f}, 1.f));
        BOOST_CHECK(!f.intersects({0.f, -10.f, 0.f}, 1.f));

        BOOST_CHECK(!f.intersects({10.f, 0.f, 0.f}, 1.f));
        BOOST_CHECK(!f.intersects({-10.f, 0.f, 0.f}, 1.f));

        BOOST_CHECK(f.intersects({10.f, 0.f, -10.f}, 1.f));
        BOOST_CHECK(f.intersects({-10.f, 0.f, -10.f}, 1.f));
    }
}

BOOST_AUTO_TEST_SUITE_END()
