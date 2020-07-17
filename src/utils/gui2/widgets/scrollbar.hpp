// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_SCROLLBAR
#define _HPP_GUI2_VIEW_SCROLLBAR

#include "SDL/SDL_ttf.h"

#include "../view.hpp"

#include "scene/objects/image2d.hpp"

namespace blunted {

  enum e_Gui2ScrollbarMode {
    e_Gui2ScrollbarMode_Horizontal,
    e_Gui2ScrollbarMode_Vertical
  };

  class Gui2Scrollbar : public Gui2View {

    public:
      Gui2Scrollbar(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Scrollbar();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Process();

      virtual void Redraw();

      void SetMode(e_Gui2ScrollbarMode mode) { this->mode = mode; }
      void SetSizePercent(float newValue) { size_percent = newValue; }
      void SetProgressPercent(float newValue) { progress_percent = newValue; }

    protected:
      boost::intrusive_ptr<Image2D> image;

      e_Gui2ScrollbarMode mode;
      float size_percent;
      float progress_percent;

  };

}

#endif
