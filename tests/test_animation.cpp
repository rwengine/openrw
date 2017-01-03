#include <boost/test/unit_test.hpp>
#include <data/Clump.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <glm/gtx/string_cast.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(AnimationTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_matrix) {
    {
        Skeleton skeleton;
        Animation animation;

        /** Models are currently needed to relate animation bones <=> model
         * frame #s. */
        auto test_model = Global::get().d->loadClump("player.dff");

        Animator animator(test_model, &skeleton);

        animation.duration = 1.f;
        animation.bones["player"] = new AnimationBone{
            "player",
            0,
            0,
            1.0f,
            AnimationBone::RT0,
            {
                {glm::quat(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(), 0.f, 0},
                {glm::quat(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(), 1.0f, 1},
            }};

        animator.playAnimation(0, &animation, 1.f, false);

        animator.tick(0.0f);

        BOOST_CHECK(skeleton.getData(0).a.translation ==
                    glm::vec3(0.f, 0.f, 0.f));
        BOOST_CHECK(skeleton.getData(0).b.translation ==
                    glm::vec3(0.f, 0.f, 0.f));

        animator.tick(1.0f);

        BOOST_CHECK(skeleton.getData(0).a.translation ==
                    glm::vec3(0.f, 1.f, 0.f));
        BOOST_CHECK(skeleton.getData(0).b.translation ==
                    glm::vec3(0.f, 0.f, 0.f));
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
