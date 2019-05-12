#include "ReverbEffect.hpp"
#include "OpenAlExtensions.hpp"
#include <efx.h>

ReverbEffect::ReverbEffect() : SoundEffect (AL_EFFECT_REVERB) {

}


void ReverbEffect::setDensity(float d) {
    alEffectf(id, AL_REVERB_DENSITY, d);
}

void ReverbEffect::setDiffusion(float d) {
    alEffectf(id, AL_REVERB_DIFFUSION, d);
}

void ReverbEffect::setGain(float g) {
    alEffectf(id, AL_REVERB_GAIN, g);
}

void ReverbEffect::setGainHf(float g) {
    alEffectf(id, AL_REVERB_GAINHF, g);
}

void ReverbEffect::setDecayTime(float t) {
    alEffectf(id, AL_REVERB_DECAY_TIME, t);
}

void ReverbEffect::setLateReverbGain(float g) {
    alEffectf(id, AL_REVERB_LATE_REVERB_GAIN, g);
}

void ReverbEffect::setLateReverbDelay(float t) {
    alEffectf(id, AL_REVERB_LATE_REVERB_DELAY, t);
}

void ReverbEffect::setAirAbsorptionGainHf(float g) {
    alEffectf(id, AL_REVERB_AIR_ABSORPTION_GAINHF, g);
}

void ReverbEffect::setDecayHfLimit(bool flag) {
    alEffecti(id, AL_REVERB_DECAY_HFLIMIT, flag ? AL_TRUE : AL_FALSE);
}
