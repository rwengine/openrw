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
    if (!created) {
        return;
    }

    alGenAuxiliaryEffectSlots(1, &slotId);

    created = alGetError() == AL_NO_ERROR;
    if (!created) {
        return;
    }

    alAuxiliaryEffectSloti(slotId, AL_EFFECTSLOT_EFFECT, (ALint) id);

    created = alGetError() == AL_NO_ERROR;
    if (!created) {
        return;
    }

    created = alGetError() == AL_NO_ERROR;
}

SoundEffect::~SoundEffect() {
    alDeleteEffects(1, &id);
    alDeleteAuxiliaryEffectSlots(1, &slotId);
}

void SoundEffect::setGain(float gain) {
    this->gain = gain;
    alAuxiliaryEffectSlotf(slotId, AL_EFFECTSLOT_GAIN, gain);
}
