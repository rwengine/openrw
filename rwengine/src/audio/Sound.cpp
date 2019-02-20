#include "audio/Sound.hpp"

#include "audio/SoundBuffer.hpp"

Sound::~Sound()
{
    if (effect != nullptr) {
        buffer->disableEffect(effect);
    }
}

bool Sound::isPlaying() const {
    return buffer->isPlaying();
}

bool Sound::isPaused() const {
    return buffer->isPaused();
}

bool Sound::isStopped() const {
    return buffer->isStopped();
}

void Sound::play() {
    buffer->play();
}

void Sound::pause() {
    buffer->pause();
}

void Sound::stop() {
    buffer->stop();
}

void Sound::setPosition(const glm::vec3 &position) {
    buffer->setPosition(position);
}

void Sound::setLooping(bool looping) {
    buffer->setLooping(looping);
}

void Sound::setPitch(float pitch) {
    buffer->setPitch(pitch);
}

void Sound::setGain(float gain) {
    buffer->setGain(gain);
}

void Sound::setMaxDistance(float maxDist) {
    buffer->setMaxDistance(maxDist);
}

void Sound::enableEffect(std::shared_ptr<SoundEffect> effect)
{
    this->effect = std::move(effect);
    buffer->enableEffect(this->effect);
}

size_t Sound::getScriptObjectID() const {
    return id;
}
