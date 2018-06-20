#include "audio/SoundManager.hpp"

#include "audio/alCheck.hpp"

//ab
#include <rw/defines.hpp>

int SoundManager::convertScriptIndexIntoSfx(const int scriptId) {
    const auto finded = std::find_if(std::begin(sfxData), std::end(sfxData),
                                     [&scriptId](const SoundInstanceData data){return data.id == scriptId;});
    return finded->sfx;
}

Sound& SoundManager::getSoundRef(const size_t& name) {
    auto ref = buffers.find(name);
    if(ref != buffers.end()) {
        return ref->second;
    } else {    // Hmm, we should reload this
        createSfxInstance(name);
    }
    return buffers[name];
}

Sound& SoundManager::getSoundRef(const std::string& name) {
    return sounds[name]; // todo reloading, how to check is it wav/mp3?
}

int SoundManager::getScriptRange(const int scriptId) {
    const auto finded = std::find_if(std::begin(sfxData), std::end(sfxData),
                                     [&scriptId](const SoundInstanceData data){return data.id == scriptId;});
    if(finded != std::end(sfxData)) {
        return finded->sfx;
    }
    return -1;
}

SoundManager::SoundManager() {
    initializeOpenAL();
    initializeAVCodec();
}

SoundManager::~SoundManager() {
    // De-initialize OpenAL
    if (alContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(alContext);
    }

    if (alDevice) alcCloseDevice(alDevice);
}

bool SoundManager::initializeOpenAL() {
    alDevice = alcOpenDevice(nullptr);
    if (!alDevice) {
        RW_ERROR("Could not find OpenAL device!");
        return false;
    }

    alContext = alcCreateContext(alDevice, nullptr);
    if (!alContext) {
        RW_ERROR("Could not create OpenAL context!");
        return false;
    }

    if (!alcMakeContextCurrent(alContext)) {
        RW_ERROR("Unable to make OpenAL context current!");
        return false;
    }

    //Needed for max distance
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

    return true;
}

bool SoundManager::initializeAVCodec() {
#if RW_DEBUG && RW_VERBOSE_DEBUG_MESSAGES
    av_log_set_level(AV_LOG_WARNING);
#else
    av_log_set_level(AV_LOG_ERROR);
#endif

    return true;
}

bool SoundManager::loadSound(const std::string& name,
                             const std::string& fileName) {
    Sound* sound = nullptr;
    auto sound_iter = sounds.find(name);

    if (sound_iter != sounds.end()) {
        sound = &sound_iter->second;
    } else {
        auto emplaced = sounds.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(name),
                                       std::forward_as_tuple());
        sound = &emplaced.first->second;

        sound->source = std::make_shared<SoundSource>();
        sound->buffer = std::make_unique<SoundBuffer>();

        sound->source->loadFromFile(fileName);
        sound->isLoaded = sound->buffer->bufferData(*sound->source);
    }

    return sound->isLoaded;
}

void SoundManager::loadSfxSounds(const std::string& path) {
    sfx.reserve(kNrOfAllSfx);
    for(size_t i = 0; i < kNrOfAllSfx; i++) {
        Sound* sound = nullptr;

        auto emplaced = sfx.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(i),
                                    std::forward_as_tuple());
        sound = &emplaced.first->second;

        sound->source = std::make_shared<SoundSource>();
        sound->source->loadSfx(path, i);

    }
}

size_t SoundManager::createSfxInstance(const size_t& index) {
    Sound* sound = nullptr;
    auto soundRef = sfx.find(index);

    //Let's try reuse buffor
    for(auto& sound : buffers) {
        if (sound.second.buffer && sound.second.isStoped())  { //Let's use this buffor
            sound.second.buffer = std::make_unique<SoundBuffer>();
            sound.second.source = soundRef->second.source;
            sound.second.isLoaded = sound.second.buffer->bufferData(*sound.second.source);
            return sound.first;
        }
    }
    // Hmmm, we should create new buffor
    auto emplaced = buffers.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(bufforNr),
                                    std::forward_as_tuple());
    sound = &emplaced.first->second;

    sound->id = bufforNr;
    sound->buffer = std::make_unique<SoundBuffer>();
    sound->source = soundRef->second.source;
    sound->isLoaded = sound->buffer->bufferData(*sound->source);
    bufforNr++;

    return sound->id;
}

bool SoundManager::isLoaded(const std::string& name) {
    auto sound = sounds.find(name);
    if (sound != sounds.end()) {
        return sound->second.isLoaded;
    }
    return false;
}

bool SoundManager::isPlaying(const std::string& name) {
    auto sound = sounds.find(name);
    if (sound != sounds.end()) {
        return sound->second.isPlaying();
    }
    return false;
}

bool SoundManager::isStoped(const std::string& name) {
    auto sound = sounds.find(name);
    if (sound != sounds.end()) {
        return sound->second.isStoped();
    }
    return false;
}

bool SoundManager::isPaused(const std::string& name) {
    auto sound = sounds.find(name);
    if (sound != sounds.end()) {
        return sound->second.isPaused();
    }
    return false;
}

void SoundManager::playSound(const std::string& name) {
    auto sound = sounds.find(name);
    if (sound != sounds.end()) {
        return sound->second.play();
    }
}

void SoundManager::playSfx(const size_t& name, const glm::vec3 position, const bool looping, const int maxDist) {
    auto buffer = buffers.find(name) ;
    if (buffer != buffers.end()) {
        buffer->second.setPosition(position);
        if(looping){
            buffer->second.setLooping(looping);
        }

        buffer->second.setPitch(1.f);
        buffer->second.setGain(1.f);
        if(maxDist != -1) {
            buffer->second.setMaxDistance(maxDist);
        }
        buffer->second.play();
    }
}

void SoundManager::pauseAllSounds() {
    for (auto &sound : sounds) {
        if(sound.second.isPlaying()) {
            sound.second.pause();
        }
    }
    for (auto &sound : buffers) {
        if(sound.second.isPlaying()) {
            sound.second.pause();
        }
    }
}

void SoundManager::resumeAllSounds() {
    for (auto &sound : sounds) {
        if(sound.second.isPaused()) {
            sound.second.play();
        }
    }
    for (auto &sound : buffers) {
        if(sound.second.isPaused()) {
            sound.second.play();
        }
    }
}

bool SoundManager::playBackground(const std::string& fileName) {
    if (this->loadSound(fileName, fileName)) {
        backgroundNoise = fileName;
        auto& sound = getSoundRef(fileName);
        sound.play();
        return true;
    }

    return false;
}

bool SoundManager::loadMusic(const std::string& name,
                             const std::string& fileName) {
    return loadSound(name, fileName);
}

void SoundManager::playMusic(const std::string& name) {
    auto sound  = sounds.find(name);
    if (sound != sounds.end()) {
        sound->second.play();
    }
}

void SoundManager::stopMusic(const std::string& name) {
    auto sound  = sounds.find(name);
    if (sound != sounds.end()) {
        sound->second.stop();
    }
}

void SoundManager::pause(bool p) {
    if (backgroundNoise.length() > 0) {
        if (p) {
            pauseAllSounds();
        } else {
            resumeAllSounds();
        }
    }
}

void SoundManager::setListenerPosition(const glm::vec3 position) {
    alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void SoundManager::setListenerVelocity(const glm::vec3 vel) {
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
}

void SoundManager::setListenerOrientation(const glm::vec3 at) {
    float v[6] = {0, at.y, 0, 0, 0, at.z};
    alListenerfv(AL_ORIENTATION, v);
}

void SoundManager::setSoundPosition(const std::string name,const glm::vec3 position) {
    if (sounds.find(name) != sounds.end()) {
        alCheck(alSource3f(sounds[name].buffer->source, AL_POSITION, position.x, position.y, position.z));
    }
}

