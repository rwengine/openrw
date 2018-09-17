# FindGLM - attempts to locate the glm matrix/vector library.
#
# This module defines the following variables (on success):
#   GLM_INCLUDE_DIRS  - where to find glm/glm.hpp
#   GLM_FOUND         - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
#   GLM_ROOT_DIR      - root directory of a glm installation
#                       Headers are expected to be found in either:
#                       <GLM_ROOT_DIR>/glm/glm.hpp           OR
#                       <GLM_ROOT_DIR>/include/glm/glm.hpp
#                       This variable can either be a cmake or environment
#                       variable. Note however that changing the value
#                       of the environment varible will NOT result in
#                       re-running the header search and therefore NOT
#                       adjust the variables set by this module.

#=============================================================================
# Copyright 2012 Carsten Neumann
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

# default search dirs
set(_glm_HEADER_SEARCH_DIRS
    "/usr/include"
    "/usr/local/include")

# check environment variable
set(_glm_ENV_ROOT_DIR "$ENV{GLM_ROOT_DIR}")

if(NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)
    set(GLM_ROOT_DIR "${_glm_ENV_ROOT_DIR}")
endif(NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)

# put user specified location at beginning of search
if(GLM_ROOT_DIR)
    set(_glm_HEADER_SEARCH_DIRS "${GLM_ROOT_DIR}"
                                "${GLM_ROOT_DIR}/include"
                                 ${_glm_HEADER_SEARCH_DIRS})
endif()

# locate header
find_path(GLM_INCLUDE_DIR "glm/glm.hpp"
    PATHS ${_glm_HEADER_SEARCH_DIRS})

if(GLM_INCLUDE_DIR)
    file(READ "${GLM_INCLUDE_DIR}/glm/detail/setup.hpp" _GLM_SETUP_HPP)
    string(REGEX MATCH "GLM: version ([0-9a-zA-Z\.\-]+)" _GLM_VERSION "${_GLM_SETUP_HPP}")
    if(NOT _GLM_VERSION)
        message(AUTHOR_WARNING "Cannot detect GLM version: regex does not match")
    endif()
    set(GLM_VERSION "${CMAKE_MATCH_1}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM
    FOUND_VAR GLM_FOUND
    REQUIRED_VARS GLM_INCLUDE_DIR
    VERSION_VAR GLM_VERSION
    )

if(GLM_FOUND)
    add_library(glm::glm INTERFACE IMPORTED)
    set_property(TARGET glm::glm
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIR})
endif()
