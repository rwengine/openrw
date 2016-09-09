#pragma once
#ifndef _LOADERGXT_HPP_
#define _LOADERGXT_HPP_
#include <data/GameTexts.hpp>
#include <platform/FileHandle.hpp>

class LoaderGXT {
public:
    void load(GameTexts& texts, FileHandle& file);
};

#endif
