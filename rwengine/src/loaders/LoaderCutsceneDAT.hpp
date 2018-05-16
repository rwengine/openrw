#ifndef _RWENGINE_LOADERCUTSCENEDAT_HPP_
#define _RWENGINE_LOADERCUTSCENEDAT_HPP_

#include <rw/forward.hpp>

struct CutsceneTracks;

class LoaderCutsceneDAT {
public:
    void load(CutsceneTracks& tracks, const FileHandle& file);
};

#endif
