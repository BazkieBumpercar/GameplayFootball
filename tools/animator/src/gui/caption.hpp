// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_CAPTION
#define _HPP_GUI_CAPTION

#include "view.hpp"
#include "scene/objects/image2d.hpp"

#include <SDL/SDL_ttf.h>

namespace blunted {

  class GuiCaption : public GuiView {

    public:
      GuiCaption(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &caption);
      virtual ~GuiCaption();

      virtual void Init();

      virtual void Set(const std::string &value);

      virtual void OnFocus();
      virtual void OnLoseFocus();
      virtual void OnKey(int sdlkID);

    protected:
      boost::intrusive_ptr<Image2D> captionImage;
      std::string caption;
      boost::intrusive_ptr < Resource<Surface> > captionResource;

      TTF_Font *font;

  };

}

#endif
