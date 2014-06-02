#include <boost/test/unit_test.hpp>
#include <engine/Animator.hpp>
#include <render/Model.hpp>
#include <glm/gtx/string_cast.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(AnimationTests)

BOOST_AUTO_TEST_CASE(test_matrix)
{
	{
		Animator animator;

		Global::get().e->gameData.loadDFF("player.dff");
		Model* test_model = Global::get().e->gameData.models["player"];

		BOOST_REQUIRE( test_model );

		Animation animation;

		animation.duration = 1.f;
		animation.bones["player"] = new AnimationBone{
				"player",
				0, 0, 1.0f,
				AnimationBone::RT0,
		{
			{
				glm::quat(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(), 0.f,
			},
			{
				glm::quat(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(), 1.0f,
			},
		}
		};

		animator.setAnimation(&animation);
		animator.setModel( test_model );

		{
			auto intp_matrix = animator.getFrameMatrixAt( test_model->frames[0], 0.0f );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 0.f, 0.f)), 0.0f );
		}

		{
			auto intp_matrix = animator.getFrameMatrixAt( test_model->frames[0], 0.5f );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( 0.0f, glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 0.5f, 0.0f)) );
		}

	}
}

BOOST_AUTO_TEST_CASE(test_interpolate)
{
	{
		Animator animator;

		Global::get().e->gameData.loadDFF("player.dff");
		Model* test_model = Global::get().e->gameData.models["player"];

		BOOST_REQUIRE( test_model );

		Animation animation;

		animation.duration = 2.0f;
		animation.bones["player"] = new AnimationBone{
				"player",
				0, 0, 2.0f,
				AnimationBone::RT0,
		{
			{
				glm::quat(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(), 0.f,
			},
			{
				glm::quat(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(), 1.0f,
			},
			{
				glm::quat(), glm::vec3(0.f, 2.f, 0.f), glm::vec3(), 2.0f,
			},
		}
		};

		animator.setAnimation(&animation);
		animator.setModel( test_model );

		{
			auto intp_matrix = animator.getFrameMatrix( test_model->frames[0] );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 0.f, 0.f)), 0.0f );
		}

		animator.tick( 1.f );

		{
			auto intp_matrix = animator.getFrameMatrix( test_model->frames[0], 0.5f );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( 0.0f, glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 0.5f, 0.0f)) );
		}

		animator.tick( 1.f );

		{
			auto intp_matrix = animator.getFrameMatrix( test_model->frames[0], 0.5f );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( 0.0f, glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 1.5f, 0.0f)) );
		}

		animator.tick( 1.f );

		{
			auto intp_matrix = animator.getFrameMatrix( test_model->frames[0], 0.5f );
			auto intp_col = intp_matrix[3];
			BOOST_CHECK_EQUAL( 0.0f, glm::distance(glm::vec3(intp_col), glm::vec3(0.f, 0.5f, 0.0f)) );
		}

	}
}

BOOST_AUTO_TEST_SUITE_END()

