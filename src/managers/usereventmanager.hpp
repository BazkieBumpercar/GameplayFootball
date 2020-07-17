// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_USEREVENT
#define _HPP_MANAGERS_USEREVENT

#include "defines.hpp"

#include "types/singleton.hpp"

#include "base/math/vector3.hpp"

#include <SDL/SDL.h>

namespace blunted {

  const int _JOYSTICK_MAX = 8;
  const int _JOYSTICK_MAXBUTTONS = 32;
  const int _JOYSTICK_MAXAXES = 8;

  struct TimedKeyPress {
    bool pressed;
    unsigned long pressTime_ms;
  };

  class UserEventManager : public Singleton<UserEventManager> {

    friend class Singleton<UserEventManager>;

    public:
      UserEventManager();
      ~UserEventManager();

      virtual void Exit();

      virtual void InputSDLEvent(const SDL_Event &event);

      bool GetKeyboardState(SDLKey key) const;
      void SetKeyboardState(SDLKey key, bool newState);
      unsigned long GetLastKeyPressDiff_ms();
      unsigned long GetLastKeyPressDiff_ms(SDLKey key);

      bool GetMouseButtonState(int sdlButtonID) const;
      Vector3 GetMouseRelativePos() const;

      int GetJoystickCount() { return SDL_NumJoysticks(); }
      bool GetJoyButtonState(int joyID, int sdlJoyButtonID) const;
      void SetJoyButtonState(int joyID, int sdlJoyButtonID, bool newState);

      float GetJoystickAxis(int joyID, int axisID, bool deadzone = true) const;
      float GetJoystickAxisRaw(int joyID, int axisID) const;
      float GetJoystickAxisCalibrationMin(int joyID, int axisID);
      float GetJoystickAxisCalibrationMax(int joyID, int axisID);
      float GetJoystickAxisCalibrationRest(int joyID, int axisID);
      void SetJoystickAxisCalibration(int joyID, int axisID, float min, float max, float rest);

    protected:
      // may need to switch to the vectors below in the future. why? SDL_keysym also takes into account unicode and modifier stuff, and contains the SDLKeys.
      // so basically, it's the parent structure, and we are going to need that info at some point, and we can't make an array with the max size (probably) since
      // i guess it has a dynamic size.
      TimedKeyPress keyPressed[SDLK_LAST];
      unsigned long lastKeyTime_ms;

      bool mousePressed[8];

      SDL_Joystick *joystick[_JOYSTICK_MAX];
      bool joyButtonPressed[_JOYSTICK_MAX][_JOYSTICK_MAXBUTTONS];
      float joyAxis[_JOYSTICK_MAX][_JOYSTICK_MAXAXES];
      float joyAxisCalibration[_JOYSTICK_MAX][_JOYSTICK_MAXAXES][3]; // min, max, rest

      mutable boost::mutex keyPressedMutex;
      mutable boost::mutex mousePressedMutex;
      mutable boost::mutex joyButtonPressedMutex;

  };

}

#endif
