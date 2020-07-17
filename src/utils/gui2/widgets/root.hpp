// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_ROOT
#define _HPP_GUI2_VIEW_ROOT

#include "../view.hpp"

namespace blunted {

  class Gui2Root : public Gui2View {

    public:
      Gui2Root(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Root();

    protected:

  };

}

#endif
