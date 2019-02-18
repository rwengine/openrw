#include "EffectSlot.hpp"
#include "SoundEffect.hpp"
#include "OpenAlExtensions.hpp"

EffectSlot::EffectSlot() {
    alGenAuxiliaryEffectSlots(1, &slotId);

    created = alGetError() == AL_NO_ERROR;
}

EffectSlot::~EffectSlot() {
    alDeleteAuxiliaryEffectSlots(1, &slotId);
}

bool EffectSlot::attachEffect(std::shared_ptr<SoundEffect> effect) {
    alAuxiliaryEffectSloti(slotId, AL_EFFECTSLOT_EFFECT, effect->getId());

    if (alGetError() != AL_NO_ERROR) {
        return false;
    }

    this->effect = std::move(effect);

    return true;
}

bool EffectSlot::detachEffect() {
    alAuxiliaryEffectSloti(slotId, AL_EFFECTSLOT_EFFECT, 0);
    this->effect = nullptr;

    return alGetError() == AL_NO_ERROR;
}

void EffectSlot::setGain(float gain) {
    alAuxiliaryEffectSlotf(slotId, AL_EFFECTSLOT_GAIN, this->gain = gain);
}
