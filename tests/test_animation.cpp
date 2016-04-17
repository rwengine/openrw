#include <boost/test/unit_test.hpp>
#include <engine/Animator.hpp>
#include <data/Skeleton.hpp>
#include <data/Model.hpp>
#include <glm/gtx/string_cast.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(AnimationTests)

BOOST_AUTO_TEST_CASE(test_matrix)
{
	{
		Skeleton skeleton;
		Animation animation;
		
		/** Models are currently needed to relate animation bones <=> model frame #s. */
		Global::get().e->data->loadDFF("player.dff");
		ModelRef& test_model = Global::get().e->data->models["player"];
		
		Animator animator(test_model->resource, &skeleton);

		animation.duration = 1.f;
		animation.bones["player"] = new AnimationBone{
				"player",
				0, 0, 1.0f,
				AnimationBone::RT0,
		{
			{
				glm::quat(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(), 0.f, 0
			},
			{
				glm::quat(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(), 1.0f, 1
			},
		}
		};
		
		animator.playAnimation(0, &animation, 1.f, true);
		
		animator.tick(0.0f);
		
		BOOST_CHECK( skeleton.getData(0).a.translation == glm::vec3(0.f, 0.f, 0.f) );
		BOOST_CHECK( skeleton.getData(0).b.translation == glm::vec3(0.f, 0.f, 0.f) );
		
		animator.tick(1.0f);
		
		BOOST_CHECK( skeleton.getData(0).a.translation == glm::vec3(0.f, 1.f, 0.f) );
		BOOST_CHECK( skeleton.getData(0).b.translation == glm::vec3(0.f, 0.f, 0.f) );
	}
}

BOOST_AUTO_TEST_SUITE_END()

