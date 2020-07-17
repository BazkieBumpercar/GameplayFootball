// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "gui.hpp"

namespace blunted {

  GuiTask::GuiTask(boost::shared_ptr<Scene2D> scene2D, float ratio, int margin) : scene2D(scene2D), ratio(ratio), margin(margin) {
    guiInterface = new GuiInterface(scene2D, ratio, margin);
  }

  GuiTask::~GuiTask() {
    scene2D.reset();
    delete guiInterface;
  }


  void GuiTask::GetPhase() {
  }

  void GuiTask::ProcessPhase() {
    guiInterface->Process();
  }

  void GuiTask::PutPhase() {
  }


  GuiInterface *GuiTask::GetInterface() {
    return guiInterface;
  }

}
