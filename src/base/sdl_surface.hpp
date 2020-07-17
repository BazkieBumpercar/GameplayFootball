// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _hpp_sdl_surface
#define _hpp_sdl_surface

#include "SDL/SDL_image.h"
#include "SDL/SDL_endian.h"

#include "SDL/sge.h"
#include "SDL/SDL_gfxPrimitives.h"

namespace blunted {

  class Triangle;

  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  static const Uint32 r_mask = 0xFF000000;
  static const Uint32 g_mask = 0x00FF0000;
  static const Uint32 b_mask = 0x0000FF00;
  static const Uint32 a_mask = 0x000000FF;
  #else
  static const Uint32 r_mask = 0x000000FF;
  static const Uint32 g_mask = 0x0000FF00;
  static const Uint32 b_mask = 0x00FF0000;
  static const Uint32 a_mask = 0xFF000000;
  #endif

  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  static const Uint8 r_mask8 = 0xC0;
  static const Uint8 g_mask8 = 0x80;
  static const Uint8 b_mask8 = 0x40;
  static const Uint8 a_mask8 = 0x00;
  #else
  static const Uint8 r_mask8 = 0x00;
  static const Uint8 g_mask8 = 0x40;
  static const Uint8 b_mask8 = 0x80;
  static const Uint8 a_mask8 = 0xC0;
  #endif

  SDL_Surface *CreateSDLSurface(int width, int height);
  void sdl_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
  Uint32 sdl_getpixel(const SDL_Surface *surface, int x, int y);
  void sdl_line(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color);
  void sdl_triangle_filled(SDL_Surface *surface, const Triangle &triangle, Uint8 r, Uint8 g, Uint8 b);
  void sdl_rectangle_filled(SDL_Surface *surface, int x, int y, int width, int height, Uint32 color);
  void sdl_alphablit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
  void sdl_flipsurface(SDL_Surface *surface);

  // only works on 32-bits surfaces
  // might have endian problems
  void sdl_setsurfacealpha(SDL_Surface *surface, int alpha);

}

#endif
