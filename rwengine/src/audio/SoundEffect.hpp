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
     * Create effect
     * @param type OpenAl specific effect type.
     */
    SoundEffect(ALint type);
    virtual ~SoundEffect();

    ALuint getId() const {
        return id;
    }

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
};

#endif // SOUNDEFFECT_H
