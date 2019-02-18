#ifndef EFFECTSLOT_H
#define EFFECTSLOT_H

#include <al.h>

#include <memory>

class SoundEffect;

/**
 * Effect slot.
 *
 * Many sound sources can be attached to one slot.
 * Different effects can be binded to this (e.g reverb, delay).
 */
class EffectSlot {
public:
    EffectSlot();
    ~EffectSlot();

    /**
     * Attach effect to this slot.
     *
     * @param sound effect (e.g reverb, delay)
     * @return true if effect attached successfully, false otherwise
     */
    bool attachEffect(std::shared_ptr<SoundEffect> effect);

    /**
     * Detach current effect from this slot.
     * @return true if effect detached successfully, false otherwise
     */
    bool detachEffect();

    void setGain(float gain);

    ALuint getSlotId() const {
        return slotId;
    }

    int getSlotNumber() const {
        return slotNumber;
    }

private:
    /**
     * Effect binded to this slot
     */
    std::shared_ptr<SoundEffect> effect;

    ALuint slotId = 0;
    /**
     * This is flag of successfull slot creation
     */
    bool created;
    float gain = 1.0f;

    /// OpenAL aux slot
    int slotNumber;
};

#endif // EFFECTSLOT_H
