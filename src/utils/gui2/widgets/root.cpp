// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "root.hpp"

namespace blunted {

  Gui2Root::Gui2Root(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {
  }

  Gui2Root::~Gui2Root() {
  }

}
