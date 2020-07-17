// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_RESOURCES_SURFACE
#define _HPP_RESOURCES_SURFACE

#include "base/sdl_surface.hpp"
#include "types/resource.hpp"

namespace blunted {

  class Surface {

    public:
      Surface();
      virtual ~Surface();
      Surface(const Surface &src);

      SDL_Surface *GetData();
      void SetData(SDL_Surface *surface);

      void Resize(int x, int y); // 0 == dependent on other coord
      void GetSize(int &x, int &y);

      void SetAlpha(float alpha);

    protected:
      SDL_Surface *surface;

  };

}

#endif
