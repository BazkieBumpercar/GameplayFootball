// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "guiinterface.hpp"

#include "managers/usereventmanager.hpp"

namespace blunted {

  GuiInterface::GuiInterface(boost::shared_ptr<Scene2D> scene2D, float ratio, int margin) : GuiView(scene2D, "root", 0, 0, 100, 100), ratio(ratio), margin(margin) {
    for (int i = 0; i < SDLK_LAST; i++) {
      keys[i] = false;
    }
  }

  GuiInterface::~GuiInterface() {
  }


  void GuiInterface::Process() {
    for (int i = 0; i < SDLK_LAST; i++) {
      bool pressed = UserEventManager::GetInstance().GetKeyboardState((SDLKey)i);
      if (pressed != keys[i]) {
        keys[i] = pressed;
        if (pressed) if (focussedView) focussedView->OnKey(i);
      }
    }
  }


  void GuiInterface::Init() {
  }


  void GuiInterface::GetCoord(float x_percent, float y_percent, int &x, int &y) {
    x = GetX(x_percent);
    y = GetY(y_percent);
  }

  int GuiInterface::GetX(float x_percent) {
    Vector3 contextSize = scene2D->GetContextSize();
    contextSize.coords[0] -= margin * 2;
    contextSize.coords[1] -= margin * 2;
    float sceneRatio = contextSize.coords[0] / contextSize.coords[1];
    float x;

    if (ratio > sceneRatio) {
      // take x as max
      x = x_percent * 0.01 * contextSize.coords[0];
    } else {
      // take y as max
      float height = contextSize.coords[1];
      float width = height * ratio;
      float offset = (contextSize.coords[0] - width) * 0.5;
      x = x_percent * 0.01 * width + offset;
    }

    return int(floor(x)) + margin;
  }

  int GuiInterface::GetY(float y_percent) {
    Vector3 contextSize = scene2D->GetContextSize();
    contextSize.coords[0] -= margin * 2;
    contextSize.coords[1] -= margin * 2;
    float sceneRatio = contextSize.coords[0] / contextSize.coords[1];
    float y;

    if (ratio > sceneRatio) {
      // take x as max
      float width = contextSize.coords[0];
      float height = width / ratio;
      float offset = (contextSize.coords[1] - height) * 0.5;
      y = y_percent * 0.01 * height + offset;
    } else {
      // take y as max
      y = y_percent * 0.01 * contextSize.coords[1];
    }

    return int(floor(y)) + margin;
  }


  void GuiInterface::OnFocus() {
  }

  void GuiInterface::OnLoseFocus() {
  }


  void GuiInterface::EmitSignal(GuiView *sender, SDLKey key) {
    GuiSignal signal;
    signal.sender = sender;
    signal.key = key;
    signalQueue.PushMessage(signal, false);
  }

}
