#include <boost/test/unit_test.hpp>
#include <WorkContext.hpp>

class TestJob : public WorkJob
{
public:
	bool *_worked, *_completed;

	TestJob( WorkContext* context, bool *w, bool *c )
		: WorkJob(context), _worked(w), _completed(c)
	{}

	void work() { *_worked = true; }

	void complete() { *_completed = true; }
};


BOOST_AUTO_TEST_SUITE(WorkTests)

BOOST_AUTO_TEST_CASE(test_interface)
{
	{
		WorkContext context;

		bool worked = false, completed = false;

		TestJob* job = new TestJob(&context, &worked, &completed);

		BOOST_CHECK( ! worked );
		BOOST_CHECK( ! completed );

		context.queueJob(job);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		BOOST_CHECK( worked );
		BOOST_CHECK( ! completed );

		context.update();;

		BOOST_CHECK( worked );
		BOOST_CHECK( completed );
	}
}

BOOST_AUTO_TEST_SUITE_END()

