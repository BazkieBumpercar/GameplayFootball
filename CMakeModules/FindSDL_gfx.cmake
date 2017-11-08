# Locate SDL_gfx library
# This module defines
# SDL_GFX_LIBRARIES, the name of the library to link against
# SDL_GFX_FOUND, if false, do not try to link to SDL_GFX
# 
# SDL_GFX_INCLUDE_DIRS, where to find sgeconfig.h
#
# $SDL_GFXDIR - enviroment variable
#
# Created by Farrer. This was influenced by the FindSDL_image.cmake module.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

FIND_PATH(SDL_GFX_INCLUDE_DIRS NAMES SDL_gfxPrimitives.h
  HINTS
  $ENV{SDL_GFXDIR}
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/sge
  /usr/local/include/SDL
  /usr/include/sge
  /usr/include/SDL
  /usr/local/include
  /usr/include
  /sw/include/sge # Fink
  /sw/include/SDL
  /sw/include
  /opt/local/include/sge # DarwinPorts
  /opt/local/include/SDL
  /opt/local/include
  /opt/csw/include/sge # Blastwave
  /opt/csw/include/SDL # Blastwave
  /opt/csw/include 
  /opt/include/sge
  /opt/include
)

FIND_LIBRARY(SDL_GFX_LIBRARIES 
   NAMES SDL_gfx
  HINTS
  $ENV{SDL_GFXDIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL_gfx
   REQUIRED_VARS SDL_GFX_LIBRARIES SDL_GFX_INCLUDE_DIRS)
