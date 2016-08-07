#pragma once
#ifndef _LOADERCUTSCENEDAT_HPP_
#define _LOADERCUTSCENEDAT_HPP_
#include <platform/FileHandle.hpp>
#include <data/CutsceneData.hpp>

class LoaderCutsceneDAT
{
public:
	void load(CutsceneTracks& tracks, FileHandle file);
};

#endif
