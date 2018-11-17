#include <iostream>

#include <boost/test/unit_test.hpp>

#include <audio/Sound.hpp>
#include <audio/SoundManager.hpp>

BOOST_AUTO_TEST_SUITE(SoundTests)

struct F {
    SoundManager manager{};
    Sound sound{};
};

BOOST_FIXTURE_TEST_CASE(creates_empty_sound, F) {
    // buffer and source should be empty
    BOOST_REQUIRE(sound.buffer == nullptr);
    BOOST_REQUIRE(sound.source == nullptr);
}

// @todo Shfil119 implement
// This test requires assets
// BOOST_AUTO_TEST_CASE(testBufferIsPlaying) {
//    sound.buffer = std::make_unique<SoundBuffer>();
//    BOOST_REQUIRE(sound.isPlaying() == false);

//    sound.play();
//    BOOST_REQUIRE(sound.isPlaying() == true);

//    sound.pause();
//    BOOST_REQUIRE(sound.isPaused() == true);

//    sound.stop();
//    BOOST_REQUIRE(sound.isStopped() == true);
//}

BOOST_FIXTURE_TEST_CASE(sound_sets_openal_source_position, F) {
    sound.buffer = std::make_unique<SoundBuffer>();

    glm::vec3 position = {100.f, 100.f, 100.f};
    sound.setPosition(position);

    float x{0.f}, y{0.f}, z{0.f};
    alGetSource3f(sound.buffer->source, AL_POSITION, &x, &y, &z);

    BOOST_REQUIRE(x == position.x);
    BOOST_REQUIRE(y == position.y);
    BOOST_REQUIRE(z == position.z);
}

BOOST_FIXTURE_TEST_CASE(sound_sets_openal_source_looping, F) {
    sound.buffer = std::make_unique<SoundBuffer>();

    // Default state should be false
    // openAL should change value of state
    int state = static_cast<bool>(true);
    alGetSourcei(sound.buffer->source, AL_LOOPING, &state);
    BOOST_REQUIRE(static_cast<bool>(state) == false);

    sound.setLooping(true);
    alGetSourcei(sound.buffer->source, AL_LOOPING, &state);
    BOOST_REQUIRE(static_cast<bool>(state) == true);
}

BOOST_FIXTURE_TEST_CASE(sound_sets_openal_source_pitch, F) {
    sound.buffer = std::make_unique<SoundBuffer>();

    sound.setPitch(0.5f);

    float pitch{0.f};
    alGetSourcef(sound.buffer->source, AL_PITCH, &pitch);

    BOOST_REQUIRE(pitch == 0.5f);
}

BOOST_FIXTURE_TEST_CASE(sound_sets_openal_source_gain, F) {
    sound.buffer = std::make_unique<SoundBuffer>();

    sound.setGain(0.5f);

    float gain{0.f};
    alGetSourcef(sound.buffer->source, AL_GAIN, &gain);

    BOOST_REQUIRE(gain == 0.5f);
}

BOOST_FIXTURE_TEST_CASE(sound_sets_openal_source_max_distance, F) {
    sound.buffer = std::make_unique<SoundBuffer>();

    sound.setMaxDistance(1000.f);

    float maxDistance{0.f};
    alGetSourcef(sound.buffer->source, AL_MAX_DISTANCE, &maxDistance);

    BOOST_REQUIRE(maxDistance == 1000.f);
}
BOOST_AUTO_TEST_SUITE_END()
