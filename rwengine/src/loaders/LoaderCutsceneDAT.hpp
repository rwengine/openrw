#pragma once
#ifndef _LOADERCUTSCENEDAT_HPP_
#define _LOADERCUTSCENEDAT_HPP_
#include <data/CutsceneData.hpp>
#include <platform/FileHandle.hpp>

class LoaderCutsceneDAT {
public:
    void load(CutsceneTracks& tracks, FileHandle file);
};

#endif
