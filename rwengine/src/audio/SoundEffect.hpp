#ifndef SOUNDEFFECT_H
#define SOUNDEFFECT_H

#include <al.h>

/**
 * Class to represent any effect.
 *
 * Any concrete realisation should have own class
 * in order to be able to set different effect-specific parameters.
 */
class SoundEffect {

public:
    /**
     * Known SoundEffect types
     */
    enum Type {
        None,
        Reverb
    };

    /**
     * Create effect
     * @param type OpenAl specific effect type.
     */
    SoundEffect(ALint type);
    virtual ~SoundEffect();

    ALuint getId() const {
        return id;
    }

    ALuint getSlotId() const {
        return slotId;
    }

    int getSlotNumber() const {
        return slotNumber;
    }

    void setGain(float gain);

protected:
    /**
     * Effect openal id
     */
    ALuint id;

private:
    /**
     * Effect created successfully if this is true after construction
     */
    bool created;

    ALuint slotId;

    float gain = 1.0f;

    /// OpenAL aux slot
    int slotNumber = 0;
};

#endif // SOUNDEFFECT_H
