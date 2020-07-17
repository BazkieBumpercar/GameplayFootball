// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_CAPTION
#define _HPP_GUI2_VIEW_CAPTION

#include "SDL/SDL_ttf.h"

#include "../view.hpp"

#include "scene/objects/image2d.hpp"

namespace blunted {

  class Gui2Caption : public Gui2View {

    public:
      Gui2Caption(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Caption();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      void SetColor(const Vector3 &color);
      void SetOutlineColor(const Vector3 &outlineColor);
      void SetTransparency(float trans);

      virtual void Redraw();

      void SetCaption(const std::string &newCaption);
      std::string GetCaption() { return caption; }

      float GetTextWidthPercent() { return textWidth_percent; }
      float GetTextWidthPercent(int subStrLength);

    protected:
      boost::intrusive_ptr<Image2D> image;

      std::string caption;
      Vector3 color;
      Vector3 outlineColor;
      float transparency;
      float textWidth_percent;
      int renderedTextHeightPix;

  };

}

#endif
