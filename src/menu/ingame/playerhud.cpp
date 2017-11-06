// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "playerhud.hpp"

#include "utils/gui2/windowmanager.hpp"

#include <SDL/SDL.h>

#include "../../onthepitch/match.hpp"

using namespace blunted;

Gui2PlayerHUD::Gui2PlayerHUD(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, Match *match) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), match(match) {
}

Gui2PlayerHUD::~Gui2PlayerHUD() {
}

void Gui2PlayerHUD::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
  target.push_back(image);
  Gui2View::GetImages(target);
}

void Gui2PlayerHUD::Redraw() {
}
