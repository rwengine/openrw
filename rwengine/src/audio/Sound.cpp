#include "audio/Sound.hpp"

#include "audio/SoundBuffer.hpp"

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

void Sound::attachToEffectSlot(const std::shared_ptr<EffectSlot> effectSlot)
{
    buffer->attachToEffectSlot(effectSlot);
}

size_t Sound::getScriptObjectID() const {
    return id;
}
