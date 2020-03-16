# Locate SDL2 library
# This module defines:
# - SDL2_LIBRARY, the name of the library to link against
# - SDL2_FOUND, if false, do not try to link to SDL2
# - SDL2_INCLUDE_DIR, where to find SDL.h
# and creates:
# - SDL2::SDL2 imported target
#
# This module responds to the the flag:
# SDL2_BUILDING_LIBRARY
# If this is defined, then no SDL2_main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL2_LIBRARY variable.
#
# SDL2_STATIC
# If this is defined, this module will search for static libraries.
# On Windows, it is uncertain whether a .lib belongs to a dynamic or static SDL2 build.
#
# Don't forget to include SDL2main.h and SDL2main.m your project for the
# OS X framework based version. (Other versions link to -lSDL2main which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
# l.e.galup  9-20-02
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL2 guidelines.
# Added a search for SDL2main which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# SDL2_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL2/SDL.h to just SDL.h
# This needed to change because "proper" SDL2 convention
# is #include "SDL.h", not <SDL2/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL2/ (see FreeBSD).
#
# Ported by Johnny Patterson. This is a literal port for SDL2 of the FindSDL.cmake
# module with the minor edit of changing "SDL" to "SDL2" where necessary. This
# was not created for redistribution, and exists temporarily pending official
# SDL2 CMake modules.

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(SDL2_INCLUDE_DIR SDL.h
    HINTS
        $ENV{SDL2DIR}
    PATH_SUFFIXES
        SDL2
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
    )

find_library(SDL2_SHARED_LIBRARY_PATH
    NAMES SDL2
    HINTS
        $ENV{SDL2DIR}
    PATH_SUFFIXES
        lib64 lib
    PATHS
        /sw
        /opt/local
        /opt/csw
        /opt
    )

find_library(SDL2_STATIC_LIBRARY_PATH
    NAMES libSDL2.lib libSDL2.a SDL2.lib SDL2.a
    HINTS
        $ENV{SDL2DIR}
    PATH_SUFFIXES
        lib64 lib
    PATHS
        /sw
        /opt/local
        /opt/csw
        /opt
    )

if(SDL2_STATIC)
    set(SDL2_LIBRARY_PATH ${SDL2_STATIC_LIBRARY_PATH})
else()
    set(SDL2_LIBRARY_PATH ${SDL2_SHARED_LIBRARY_PATH})
endif()

find_library(SDL2_SDL2MAIN_LIBRARY_PATH
    NAMES libSDL2main.lib libSDL2main.a SDL2main.lib SDL2main.a
    HINTS
        $ENV{SDL2DIR}
    PATH_SUFFIXES
        lib64 lib
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include/SDL2
        /usr/include/SDL2
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
    )

if(SDL2_INCLUDE_DIR)
    file(READ "${SDL2_INCLUDE_DIR}/SDL_version.h" _SDL_VERSION_H)
    set(SDL2_REGEXES
        "SDL_MAJOR_VERSION[ ]+([0-9]+)"
        "SDL_MINOR_VERSION[ ]+([0-9]+)"
        "SDL_PATCHLEVEL[ ]+([0-9]+)"
        )
    set(_SDL2_VERSION)
    foreach(_SDL2_REGEX ${SDL2_REGEXES})
        string(REGEX MATCH "${_SDL2_REGEX}" _SDL2_NUMBER "${_SDL_VERSION_H}")
        if(NOT _SDL2_NUMBER)
            message(AUTHOR_WARNING "Cannot detect SDL2 version: regex \"${_SDL2_REGEX}\" does not match")
        endif()
        list(APPEND _SDL2_VERSION "${CMAKE_MATCH_1}")
    endforeach()
    string(REPLACE ";" "." SDL2_VERSION "${_SDL2_VERSION}")
endif()

set(SDL2_DEPENDENCIES)

# For SDLmain
if(SDL2_STATIC)
    list(APPEND SDL2_DEPENDENCIES ${CMAKE_DL_LIBS})
endif()

if(SDL2_STATIC AND NOT SDL2_BUILDING_LIBRARY)
    list(APPEND SDL2_DEPENDENCIES ${SDL2_SDL2MAIN_LIBRARY_PATH})
endif()

# For OS X, SDL2 uses Cocoa as a backend so it must link to Cocoa.
# CMake doesn't display the -framework Cocoa string in the UI even
# though it actually is there if I modify a pre-used variable.
# I think it has something to do with the CACHE STRING.
# So I use a temporary variable until the end so I can set the
# "real" variable in one-shot.
if(APPLE)
    list(APPEND SDL2_DEPENDENCIES "-framework Cocoa")
endif()

# SDL2 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
# In fact, there seems to be a problem if I used the Threads package
# and try using this line, so I'm just skipping it entirely for OS X.
if(NOT APPLE)
    find_package(Threads REQUIRED)
    list(APPEND SDL2_DEPENDENCIES Threads::Threads)
endif()

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDL2main -lSDL2 -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
if(MINGW)
    set(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    list(APPEND SDL2_DEPENDENCIES ${MINGW32_LIBRARY})
endif()

if(WIN32)
    list(APPEND SDL2_DEPENDENCIES winmm imm32 version msimg32)
endif()

set(SDL2_LIBRARY ${SDL2_LIBRARY_PATH} ${SDL2_DEPENDENCIES})

set(SDL2_REQUIRED_LIBS SDL2_LIBRARY_PATH)
if(NOT SDL2_BUILDING_LIBRARY)
    list(APPEND SDL2_REQUIRED_LIBS SDL2_SDL2MAIN_LIBRARY_PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    FOUND_VAR SDL2_FOUND
    REQUIRED_VARS ${SDL2_REQUIRED_LIBS} SDL2_INCLUDE_DIR
    VERSION_VAR SDL2_VERSION
    )

if(SDL2_FOUND)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_property(TARGET SDL2::SDL2
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR})
    set_property(TARGET SDL2::SDL2
        PROPERTY INTERFACE_LINK_LIBRARIES ${SDL2_LIBRARY})
endif()
