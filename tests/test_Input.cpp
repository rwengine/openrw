#include <GameInput.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(InputTests)

BOOST_AUTO_TEST_CASE(TestStateUpdate) {
    BOOST_CHECK(GameInputState::Handbrake != GameInputState::Submission);
    BOOST_CHECK(GameInputState::Jump != GameInputState::Sprint);
    {
        // Currently tests against hard-coded input
        GameInputState state;

        SDL_Event ev;
        ev.type = SDL_KEYDOWN;
        ev.key.keysym.sym = SDLK_SPACE;

        GameInput::updateGameInputState(&state, ev);

        // Check that the correct inputs report pressed
        for (int c = 0; c < GameInputState::_MaxControls; ++c) {
            switch (static_cast<GameInputState::Control>(c)) {
                case GameInputState::Jump:
                case GameInputState::Handbrake:
                    BOOST_CHECK(
                        state.pressed(static_cast<GameInputState::Control>(c)));
                    break;
                default:
                    BOOST_CHECK(!state.pressed(
                        static_cast<GameInputState::Control>(c)));
                    break;
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
