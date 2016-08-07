#pragma once
#ifndef _LOADERGXT_HPP_
#define _LOADERGXT_HPP_
#include <platform/FileHandle.hpp>
#include <data/GameTexts.hpp>

class LoaderGXT
{
public:
	void load( GameTexts& texts, FileHandle& file );
};

#endif
