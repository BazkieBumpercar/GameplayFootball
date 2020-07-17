// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_IMAGE
#define _HPP_GUI2_VIEW_IMAGE

#include "../view.hpp"

#include "scene/objects/image2d.hpp"

namespace blunted {

  class Gui2Image : public Gui2View {

    public:
      Gui2Image(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Image();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      void LoadImage(const std::string &filename);
      virtual void Redraw();

      virtual void SetSize(float new_width_percent, float new_height_percent);
      virtual void SetZoom(float zoomx, float zoomy);

      boost::intrusive_ptr<Image2D> &GetImage2D() { return image; }

    protected:
      boost::intrusive_ptr<Image2D> image;
      boost::intrusive_ptr<Image2D> imageSource;

  };

}

#endif
