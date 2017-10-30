#pragma once
#ifndef _LOADERCUTSCENEDAT_HPP_
#define _LOADERCUTSCENEDAT_HPP_

#include <data/forward.hpp>
#include <rw/forward.hpp>

class LoaderCutsceneDAT {
public:
    void load(CutsceneTracks& tracks, FileHandle file);
};

#endif
