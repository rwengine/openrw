#ifndef _RWENGINE_LOADERGXT_HPP_
#define _RWENGINE_LOADERGXT_HPP_
#include <rw/forward.hpp>

class GameTexts;

class LoaderGXT {
public:
    void load(GameTexts& texts, const FileHandle& file);
};

#endif
