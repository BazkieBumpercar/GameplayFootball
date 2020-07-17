// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_EVENTS
#define _HPP_GUI2_EVENTS

#include "base/math/vector3.hpp"
#include "managers/usereventmanager.hpp"
#include <SDL/SDL.h>

namespace blunted {

  enum e_Gui2EventType {
    e_Gui2EventType_Windowing = 0,
    e_Gui2EventType_Mouse = 1,
    e_Gui2EventType_Keyboard = 2,
    e_Gui2EventType_Joystick = 3,
    e_Gui2EventType_User = 100
  };

  class Gui2Event {

    public:
      Gui2Event(e_Gui2EventType eventType);
      virtual ~Gui2Event();

      e_Gui2EventType GetType() const;

      void Accept() { accepted = true; }
      void Ignore() { accepted = false; }

      bool IsAccepted() const { return accepted; }

    protected:
      e_Gui2EventType eventType;

      bool accepted;

  };

  class WindowingEvent : public Gui2Event {

    public:
      WindowingEvent();
      virtual ~WindowingEvent();

      bool IsActivate() { return activate; }
      bool IsEscape() { return escape; }
      Vector3 GetDirection() { return direction; }

      void SetActivate() { activate = true; }
      void SetEscape() { escape = true; }
      void SetDirection(const Vector3 &direction) { this->direction = direction; }

    protected:
      bool activate;
      bool escape;
      Vector3 direction;

  };

  class KeyboardEvent : public Gui2Event {

    public:
      KeyboardEvent();
      virtual ~KeyboardEvent();

      bool GetKeyOnce(int id) const { return keyOnce[id]; }
      void SetKeyOnce(int id) { keyOnce[id] = true; }
      bool GetKeyContinuous(int id) const { return keyContinuous[id]; }
      void SetKeyContinuous(int id) { keyContinuous[id] = true; }
      bool GetKeyRepeated(int id) const { return keyRepeated[id]; }
      void SetKeyRepeated(int id) { keyRepeated[id] = true; }
      std::vector<SDL_keysym> &GetKeysymOnce() { return keysymOnce; }
      std::vector<SDL_keysym> &GetKeysymContinuous() { return keysymContinuous; }
      std::vector<SDL_keysym> &GetKeysymRepeated() { return keysymRepeated; }

    protected:
      std::vector<SDL_keysym> keysymOnce;
      std::vector<SDL_keysym> keysymContinuous;
      std::vector<SDL_keysym> keysymRepeated;

      bool keyOnce[SDLK_LAST];
      bool keyContinuous[SDLK_LAST];
      bool keyRepeated[SDLK_LAST];

  };

  class JoystickEvent : public Gui2Event {

    public:
      JoystickEvent();
      virtual ~JoystickEvent();

      bool GetButton(int joyID, int id) const { return button[joyID][id]; }
      void SetButton(int joyID, int id) { button[joyID][id] = true; }
      float GetAxis(int joyID, int id) const { return axes[joyID][id]; }
      void SetAxis(int joyID, int id, float value) { this->axes[joyID][id] = value; }

    protected:
      // todo: doesn't it make more sense to have one joystick event per joyid? that's what we're actually doing, but then why have this multidimensional array and not just a seperate joyid variable?
      bool button[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      float axes[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];

  };

}

#endif
