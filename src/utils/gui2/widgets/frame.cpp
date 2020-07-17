// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "frame.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2Frame::Gui2Frame(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, bool background) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {
    if (background) {
      Gui2Image *bg = new Gui2Image(windowManager, name + "_frame", 0, 0, width_percent, height_percent);
      this->AddView(bg);
      bg->LoadImage("media/menu/backgrounds/black.png");
      bg->Show();
    }
  }

  Gui2Frame::~Gui2Frame() {
  }

}
