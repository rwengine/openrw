#ifndef REVERBEFFECT_H
#define REVERBEFFECT_H

#include "SoundEffect.hpp"

class ReverbEffect : public SoundEffect {

public:
    ReverbEffect();

    void setDensity(float density = 1.0f);
    void setDiffusion(float diffusion = 0.3f);
    void setGain(float gain = 0.92f);
    void setGainHf(float gainHf = 0.89f);
    void setDecayTime(float decayTime = 5.49f);
    void setLateReverbGain(float lateReverbGain = 1.26f );
    void setLateReverbDelay(float lateReverbDelay = 0.011f);
    void setAirAbsorptionGainHf(float absorptionGainHf = 0.994f);
    void setDecayHfLimit(bool decayHfLimit = true);
};

#endif // REVERBEFFECT_H
