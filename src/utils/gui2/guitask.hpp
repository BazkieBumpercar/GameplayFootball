// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_GUI2TASK
#define _HPP_GUI2_GUI2TASK

#include <SDL/SDL_ttf.h>

#include "types/iusertask.hpp"

#include "scene/scene2d/scene2d.hpp"

#include "windowmanager.hpp"

namespace blunted {

  class Gui2Task : public IUserTask {

    public:
      Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin);
      virtual ~Gui2Task();

      virtual void GetPhase();
      virtual void ProcessPhase();
      virtual void PutPhase();

      virtual void ProcessEvents();

      Gui2WindowManager *GetWindowManager() { return this->windowManager; }

      void SetEventJoyButtons(int activate, int escape);
      void GetEventJoyButtons(int &activate, int &escape) { activate = joyButtonActivate; escape = joyButtonEscape; }

      void EnableKeyboard() { keyboard = true; }
      void DisableKeyboard() { keyboard = false; }
      bool IsKeyboardActive() { return keyboard; }
      void SetActiveJoystickID(int joyID) { activeJoystick = joyID; }
      int GetActiveJoystickID() const { return activeJoystick; }

      virtual std::string GetName() const { return "gui2"; }

    protected:
      boost::shared_ptr<Scene2D> scene2D;

      Gui2WindowManager *windowManager;

      bool prevKeyState[SDLK_LAST];
      bool prevButtonState[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      bool prevAxisState[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];

      int joyButtonActivate;
      int joyButtonEscape;
      int activeJoystick;
      bool keyboard;

  };

}

#endif
