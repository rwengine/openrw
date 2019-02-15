#include "SoundEffect.hpp"
#include "OpenAlExtensions.hpp"

SoundEffect::SoundEffect(ALint type) {
    // reset error
    alGetError();

    alGenEffects(1, &id);

    created = alGetError() == AL_NO_ERROR;
    if (!created) {
        return;
    }

    alEffecti(id, AL_EFFECT_TYPE, type);

    created = alGetError() == AL_NO_ERROR;
}

SoundEffect::~SoundEffect() {
    alDeleteEffects(1, &id);
}
