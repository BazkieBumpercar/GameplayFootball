// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_CAPTUREKEY
#define _HPP_GUI2_VIEW_CAPTUREKEY

#include "../view.hpp"

namespace blunted {

  class Gui2CaptureKey : public Gui2View {

    public:
      Gui2CaptureKey(Gui2WindowManager *windowManager, const std::string &name);
      virtual ~Gui2CaptureKey();

      virtual void ProcessWindowingEvent(WindowingEvent *event) { event->Accept(); }
      virtual void ProcessKeyboardEvent(KeyboardEvent *event);
      int GetKeyID();

      boost::signals2::signal<void(Gui2CaptureKey*)> sig_OnKey;

    protected:
      SDL_Keycode keyID;

  };

  enum e_JoystickInputType {
    e_JoystickInputType_Button,
    e_JoystickInputType_Axis
  };

  class Gui2CaptureJoy : public Gui2View {

    public:
      Gui2CaptureJoy(Gui2WindowManager *windowManager, const std::string &name, int controllerID);
      virtual ~Gui2CaptureJoy();

      virtual void ProcessWindowingEvent(WindowingEvent *event) { event->Accept(); }
      virtual void ProcessKeyboardEvent(KeyboardEvent *event); // for escaping
      virtual void ProcessJoystickEvent(JoystickEvent *event);
      e_JoystickInputType GetInputType() { return inputType; }
      int GetButtonID();
      int GetAxisID();
      signed int GetAxisSign();

      boost::signals2::signal<void(Gui2CaptureJoy*)> sig_OnJoy;

    protected:
      int controllerID;
      e_JoystickInputType inputType;
      int buttonID;
      signed int axisID;
      signed int axisSign;
      signed int tmpAxisID;
      signed int tmpAxisSign;

  };

}

#endif
