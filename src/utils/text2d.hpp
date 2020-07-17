// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

/* DEPRECATED CLASS */

#ifndef _HPP_UTILS_TEXT2D
#define _HPP_UTILS_TEXT2D

#include "defines.hpp"

#include "scene/objects/image2d.hpp"

#include "scene/scene2d/scene2d.hpp"

#include <SDL/SDL_ttf.h>

namespace blunted {

  class Text2D {

    public:
      Text2D(boost::shared_ptr<Scene2D> scene2D, const std::string &fontFile = "c:/windows/fonts/cour.ttf", int pts = 14);
      ~Text2D();

      boost::intrusive_ptr<Image2D> Create(int width);
      void SetText(boost::intrusive_ptr<Image2D> image, const std::string &text, const Vector3 &color) const;

    protected:
      SDL_Surface *RenderTextSurface(const std::string &text, const Vector3 &color) const;

      TTF_Font *font;
      mutable int count;

      boost::shared_ptr<Scene2D> scene2D;

  };

}

#endif
