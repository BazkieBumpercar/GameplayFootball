// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "usereventmanager.hpp"

#include "environmentmanager.hpp"

namespace blunted {

  template<> UserEventManager* Singleton<UserEventManager>::singleton = 0;

  UserEventManager::UserEventManager() {
    lastKeyTime_ms = 0;

    SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);

    for (int i = 0; i < SDLK_LAST; i++) {
      keyPressed[i].pressed = false;
      keyPressed[i].pressTime_ms = 0;
    }

    // yes, SDL starts mousebuttons at 1...
    for (int i = 1; i < 8; i++) {
      mousePressed[i] = false;
    }

    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
        joyButtonPressed[j][i] = false;
      }
    }

    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
        joyAxis[j][i] = 0.0;
        joyAxisCalibration[j][i][0] = -32768.0;
        joyAxisCalibration[j][i][1] = 32767.0;
        joyAxisCalibration[j][i][2] = 0.0;
      }
    }


    // init the joy!

    SDL_Init(SDL_INIT_JOYSTICK);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
      joystick[i] = SDL_JoystickOpen(i);
    }
    //SDL_JoystickEventState(SDL_IGNORE); // doesn't seem to work? bug?
    SDL_JoystickEventState(SDL_ENABLE);
    //printf("JOYSTICK EVENT STATE: %i (%i = ignore, %i = enable)\n", SDL_JoystickEventState(SDL_QUERY), SDL_IGNORE, SDL_ENABLE);
  }

  UserEventManager::~UserEventManager() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
      SDL_JoystickClose(joystick[i]);
    }
  }

  void UserEventManager::Exit() {
  }

  void UserEventManager::InputSDLEvent(const SDL_Event &event) {
    int joyID = 0;
    switch (event.type) {
      case SDL_KEYDOWN:
        keyPressedMutex.lock();
        keyPressed[event.key.keysym.sym].pressed = true;
        keyPressed[event.key.keysym.sym].pressTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
        lastKeyTime_ms = keyPressed[event.key.keysym.sym].pressTime_ms;
        keyPressedMutex.unlock();
        break;
      case SDL_KEYUP:
        keyPressedMutex.lock();
        keyPressed[event.key.keysym.sym].pressed = false;
        keyPressedMutex.unlock();
        break;
      case SDL_MOUSEBUTTONDOWN:
        mousePressedMutex.lock();
        mousePressed[event.button.button] = true;
        mousePressedMutex.unlock();
        break;
      case SDL_MOUSEBUTTONUP:
        mousePressedMutex.lock();
        mousePressed[event.button.button] = false;
        mousePressedMutex.unlock();
        break;
      case SDL_JOYAXISMOTION:
        joyButtonPressedMutex.lock();
        joyAxis[event.jaxis.which][event.jaxis.axis] = event.jaxis.value;
        joyButtonPressedMutex.unlock();
        break;
      case SDL_JOYBUTTONDOWN:
        joyButtonPressedMutex.lock();
        joyID = event.jaxis.which;
        joyButtonPressed[joyID][event.jbutton.button] = true;
        joyButtonPressedMutex.unlock();
        break;
      case SDL_JOYBUTTONUP:
        joyButtonPressedMutex.lock();
        joyID = event.jaxis.which;
        joyButtonPressed[joyID][event.jbutton.button] = false;
        joyButtonPressedMutex.unlock();
        break;
    }
  }

  bool UserEventManager::GetKeyboardState(SDLKey key) const {
    boost::mutex::scoped_lock lock(keyPressedMutex);
    return keyPressed[key].pressed;
  }

  void UserEventManager::SetKeyboardState(SDLKey key, bool newState) {
    boost::mutex::scoped_lock lock(keyPressedMutex);
    keyPressed[key].pressed = newState;
    if (newState) keyPressed[key].pressTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
  }

  unsigned long UserEventManager::GetLastKeyPressDiff_ms() {
    boost::mutex::scoped_lock lock(keyPressedMutex);
    return EnvironmentManager::GetInstance().GetTime_ms() - lastKeyTime_ms;
  }

  unsigned long UserEventManager::GetLastKeyPressDiff_ms(SDLKey key) {
    boost::mutex::scoped_lock lock(keyPressedMutex);
    return EnvironmentManager::GetInstance().GetTime_ms() - keyPressed[key].pressTime_ms;
  }

  bool UserEventManager::GetMouseButtonState(int sdlButtonID) const {
    boost::mutex::scoped_lock lock(mousePressedMutex);
    return mousePressed[sdlButtonID];
  }

  Vector3 UserEventManager::GetMouseRelativePos() const {
    Vector3 mousePos;
    mousePos.coords[2] = 0;
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    mousePos.coords[0] = x;
    mousePos.coords[1] = y;
    return mousePos;
  }


  bool UserEventManager::GetJoyButtonState(int joyID, int sdlJoyButtonID) const {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    return joyButtonPressed[joyID][sdlJoyButtonID];
  }

  void UserEventManager::SetJoyButtonState(int joyID, int sdlJoyButtonID, bool newState) {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    joyButtonPressed[joyID][sdlJoyButtonID] = newState;
  }

  float UserEventManager::GetJoystickAxis(int joyID, int axisID, bool deadzone) const {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);

    float min = joyAxisCalibration[joyID][axisID][0];
    float max = joyAxisCalibration[joyID][axisID][1];
    float rest = joyAxisCalibration[joyID][axisID][2];

    float value = joyAxis[joyID][axisID];

    if (value < min) value = min;
    if (value > max) value = max;
    float scale = max - min;
    if (scale == 0.0) scale = 0.01; // avoid division by zero, axis would be defunct though if scale evaluates to 0

    // bring value in range 0 .. 1
    value -= min;
    value /= scale;

    // bring rest in range 0 .. 1
    rest -= min;
    rest /= scale;

    // deadzone
    if (deadzone)
      if (fabs(rest - value) < 0.1) value = rest;

    if (value < rest) {
      // bring value in range 0 .. -1
      value /= rest;
      value -= 1.0;
    } else if (value > rest) {
      // bring value in range 0 .. 1
      scale = 1.0 - rest;
      value -= rest;
      value /= scale;
    } else { // value == rest
      value = 0.0;
    }

    return value;
  }

  float UserEventManager::GetJoystickAxisRaw(int joyID, int axisID) const {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    return joyAxis[joyID][axisID];
  }

  float UserEventManager::GetJoystickAxisCalibrationMin(int joyID, int axisID) {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    return joyAxisCalibration[joyID][axisID][0];
  }

  float UserEventManager::GetJoystickAxisCalibrationMax(int joyID, int axisID) {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    return joyAxisCalibration[joyID][axisID][1];
  }

  float UserEventManager::GetJoystickAxisCalibrationRest(int joyID, int axisID) {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    return joyAxisCalibration[joyID][axisID][2];
  }

  void UserEventManager::SetJoystickAxisCalibration(int joyID, int axisID, float min, float max, float rest) {
    boost::mutex::scoped_lock lock(joyButtonPressedMutex);
    joyAxisCalibration[joyID][axisID][0] = min;
    joyAxisCalibration[joyID][axisID][1] = max;
    joyAxisCalibration[joyID][axisID][2] = rest;

    // rest has to be within min/max range
    if (joyAxisCalibration[joyID][axisID][2] < joyAxisCalibration[joyID][axisID][0]) joyAxisCalibration[joyID][axisID][2] = joyAxisCalibration[joyID][axisID][0];
    if (joyAxisCalibration[joyID][axisID][2] > joyAxisCalibration[joyID][axisID][1]) joyAxisCalibration[joyID][axisID][2] = joyAxisCalibration[joyID][axisID][1];
    joyAxis[joyID][axisID] = rest;
  }

}
