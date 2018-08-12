#ifndef _RWENGINE_SFX_PARAMETERS_HPP_
#define _RWENGINE_SFX_PARAMETERS_HPP_

#include <cstddef>

/// Script is using different numeration of sounds
/// than postion index in sfx file.
/// Also it is needed to store range of sound.
/// Struct is used by opcodes 018c, 018d.
struct SoundInstanceData {
    int id;
    int sfx;
    int range;
};

/// Get metadata for selected script index
const SoundInstanceData* getSoundInstanceData(int scriptId);

#endif
