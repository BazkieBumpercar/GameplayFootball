#ifndef _SDLUTILS_H_
#define _SDLUTILS_H_
/*
    NET2 is a threaded, event based, network IO library for SDL.
    Copyright (C) 2002 Bob Pendleton

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation; either version 2.1
    of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    If you do not wish to comply with the terms of the LGPL please
    contact the author as other terms are available for a fee.
    
    Bob Pendleton
    Bob@Pendleton.com
*/

#include "SDL.h"
#include "SDL_net.h"

#ifdef __cplusplus
extern "C" {
#endif

  void printSDLColor(const SDL_Color *c);
  void printSDLEvent(SDL_Event *e);
  void printSDLInitFlags(const Uint32 f);
  void printSDLModifiers(SDLMod mod);
  void printSDLOpenGLAttrs();
  void printSDLPalette(const SDL_Palette *p);
  void printSDLPixelFormat(const SDL_PixelFormat *f);
  void printSDLRect(const SDL_Rect *r);
  void printSDLSurface(const SDL_Surface *s);
  void printSDLSurfaceFlags(const Uint32 f);
  void printSDLVideoInfo(const SDL_VideoInfo *vi);
  void printIPaddress(IPaddress *addr);  // IPaddress structure print utility
  void printNET2Event(SDL_Event *event); // net2 event print utility
  void printUDPpacket(UDPpacket *packet);
  void printBytes(char *buf, int len);

  void mySDLInitOrQuit(Uint32 flags);   // Initialize SDL and networking or die trying
  char *mySDL_Init(Uint32 flags);       // Initialize networking and the rest of SDL
  void mySDL_Quit();                    // Quit networking and the rest of SDL

#ifdef __cplusplus
}
#endif

#endif
