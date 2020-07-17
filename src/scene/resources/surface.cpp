// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "surface.hpp"

#include "base/log.hpp"

#include "SDL/SDL_rotozoom.h"

namespace blunted {

  Surface::Surface() : surface(0) {
    //printf("CREATING SURFACE\n");
  }

  Surface::~Surface() {
    //printf("ANNIHILATING SURFACE.. ");
    if (surface) {
      SDL_FreeSurface(surface);
      surface = 0;
    }
  }

  Surface::Surface(const Surface &src) {
    this->surface = SDL_ConvertSurface(src.surface, src.surface->format, 0);
    assert(this->surface);
  }

  SDL_Surface *Surface::GetData() {
    return surface;
  }

  void Surface::SetData(SDL_Surface *surface) {
    if (this->surface) SDL_FreeSurface(this->surface);
    this->surface = surface;
  }

  void Surface::Resize(int x, int y) {

    // zoomSurface doesn't seem to create a completely new surface; got some weird segfaults.
    // not 100% sure if it's their fault though, or if i'm doing something wrong. either way,
    // it works with this fix, though it's a bit of a performance hit, an extra surface copy.
    bool buggyZoomSurface = true;

    assert(this->surface);
    int xcur = this->surface->w;
    int ycur = this->surface->h;
    double xfac, yfac;
    xfac = x / (xcur * 1.0);
    yfac = y / (ycur * 1.0);
    if (yfac == 0) yfac = xfac;
    if (xfac == 0) xfac = yfac;
    if (xfac == 0 || yfac == 0) return;
    SDL_Surface *newSurf = zoomSurface(this->surface, xfac, yfac, SMOOTHING_ON);
    //printf("resize factors: %f %f\n", xfac, yfac);
    //printf("surface size: %i %i\n", this->surface->w, this->surface->h);
    //printf("new surface size: %i %i\n", newSurf->w, newSurf->h);
    SDL_FreeSurface(this->surface);
    if (buggyZoomSurface) {
      this->surface = SDL_ConvertSurface(newSurf, newSurf->format, 0);
      SDL_FreeSurface(newSurf);
    } else {
      this->surface = newSurf;
    }
  }

  void Surface::GetSize(int &x, int &y) {
    x = this->surface->w;
    y = this->surface->h;
  }

  void Surface::SetAlpha(float alpha) {
    sdl_setsurfacealpha(this->surface, int(round(alpha * 255)));
  }

}
