// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_INTERFACE
#define _HPP_GUI_INTERFACE

#include "defines.hpp"
#include "scene/scene2d/scene2d.hpp"

#include "button.hpp"
#include "timeline.hpp"
#include "filedialog.hpp"
#include "textinput.hpp"

#include "view.hpp"

namespace blunted {

  struct GuiSignal {
    GuiSignal() {
      sender = 0;
      key = SDLK_0;
    }
    GuiView *sender;
    SDLKey key;
  };

  class GuiInterface : public GuiView {

    public:
      GuiInterface(boost::shared_ptr<Scene2D> scene2D, float ratio, int margin);
      virtual ~GuiInterface();

      void Process();

      virtual void Init();

      virtual void GetCoord(float x_percent, float y_percent, int &x, int &y);
      virtual int GetX(float x_percent);
      virtual int GetY(float y_percent);

      virtual void OnFocus();
      virtual void OnLoseFocus();

      virtual void EmitSignal(GuiView *sender, SDLKey key);

      MessageQueue<GuiSignal> signalQueue;

    protected:
      float ratio;
      int margin;

      bool keys[SDLK_LAST];

  };

}

#endif
