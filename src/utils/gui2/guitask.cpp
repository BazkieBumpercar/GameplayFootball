// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "guitask.hpp"

#include "managers/usereventmanager.hpp"

namespace blunted {

  Gui2Task::Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin) : scene2D(scene2D) {
    windowManager = new Gui2WindowManager(scene2D, aspectRatio, margin);
    for (int i = 0; i < SDLK_LAST; i++) {
      prevKeyState[i] = false;
    }
    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
        prevButtonState[j][i] = false;
      }
    }
    for (int j = 0; j < _JOYSTICK_MAX; j++) {
      for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
        prevAxisState[j][i] = false;
      }
    }

    joyButtonActivate = 1;
    joyButtonEscape = 1;
    activeJoystick = 0;

    keyboard = true;
  }

  Gui2Task::~Gui2Task() {
    windowManager->Exit();
    delete windowManager;
  }

  void Gui2Task::GetPhase() {
  }

  void Gui2Task::ProcessPhase() {
    windowManager->GetRoot()->SetRecursiveZPriority(0); // todo: this can probably be optimized (like, doing it every frame seems redundant?)
    //windowManager->GetRoot()->PrintTree(0);
    ProcessEvents();
    windowManager->Process();
  }

  void Gui2Task::PutPhase() {
  }

  void Gui2Task::ProcessEvents() {

    // todo: stop making objects every process(); this is terribly inefficient. find out if they are needed in the first place.

    if (!windowManager->GetFocus()) return;
    Gui2View *currentFocus = windowManager->GetFocus();

    // some buttons need to be sent through a windowing event (escape, enter, directional controls)
    // if a view accepts a keyboard/joystick event, don't send windowing event => *disabled, now uses focus check
    bool needsWindowingEvent = false;
    WindowingEvent *wEvent = new WindowingEvent();


    // keyboard events

    KeyboardEvent *event = new KeyboardEvent();
    bool needsKeyboardEvent = false;

    for (int i = 0; i < SDLK_LAST; i++) {

      bool pressed = UserEventManager::GetInstance().GetKeyboardState((SDLKey)i);

      // continuous
      if (pressed) {

        event->SetKeyContinuous(i);
        SDL_keysym tmp; // in future: use keysyms instead of keys. see commentary in usereventmanager.hpp
        tmp.sym = (SDLKey)i;
        event->GetKeysymContinuous().push_back(tmp);
        //printf("key %i pressed\n", i);

        needsKeyboardEvent = true;
      }

      // only when pressed first time
      if (pressed && !prevKeyState[i]) {

        event->SetKeyOnce(i);
        SDL_keysym tmp; // in future: use keysyms instead of keys. see commentary in usereventmanager.hpp
        tmp.sym = (SDLKey)i;
        event->GetKeysymOnce().push_back(tmp);

        if (keyboard) {
          if ((SDLKey)i == SDLK_RETURN) {
            wEvent->SetActivate();
            needsWindowingEvent = true;
          }
          else if ((SDLKey)i == SDLK_ESCAPE) {
            wEvent->SetEscape();
            needsWindowingEvent = true;
          }
        }

        needsKeyboardEvent = true;
      }

      // when pressed first time, or when held for longer than some treshold
      if (pressed && (!prevKeyState[i] || UserEventManager::GetInstance().GetLastKeyPressDiff_ms((SDLKey)i) > 250)) {

        event->SetKeyRepeated(i);
        SDL_keysym tmp; // in future: use keysyms instead of keys. see commentary in usereventmanager.hpp
        tmp.sym = (SDLKey)i;
        event->GetKeysymRepeated().push_back(tmp);

        if (keyboard) {
          if ((SDLKey)i == SDLK_UP) {
            wEvent->SetDirection(Vector3(0, -1, 0));
            needsWindowingEvent = true;
          }
          else if ((SDLKey)i == SDLK_RIGHT) {
            wEvent->SetDirection(Vector3(1, 0, 0));
            needsWindowingEvent = true;
          }
          else if ((SDLKey)i == SDLK_DOWN) {
            wEvent->SetDirection(Vector3(0, 1, 0));
            needsWindowingEvent = true;
          }
          else if ((SDLKey)i == SDLK_LEFT) {
            wEvent->SetDirection(Vector3(-1, 0, 0));
            needsWindowingEvent = true;
          }
        }

        needsKeyboardEvent = true;
      }

      prevKeyState[i] = pressed;
    }

    if (needsKeyboardEvent) {
      if (windowManager->GetFocus() == currentFocus) windowManager->GetFocus()->ProcessEvent(event);
    }
    delete event;


    // joystick events

    for (int j = 0; j < UserEventManager::GetInstance().GetJoystickCount(); j++) {

      float axes[_JOYSTICK_MAXAXES];
      bool stick = false;
      for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
        axes[i] = UserEventManager::GetInstance().GetJoystickAxis(j, i);
        if (fabs(axes[i]) > 0.2) {
          stick = true;
          prevAxisState[j][i] = true;
        } else {
          axes[i] = 0.0;
          if (prevAxisState[j][i] == true) stick = true; // report axis back-to-middle
          prevAxisState[j][i] = false;
        }
      }

      bool buttonChanged = false;
      for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
        if (UserEventManager::GetInstance().GetJoyButtonState(j, i) != prevButtonState[j][i]) {
          buttonChanged = true;
          break;
        }
      }

      if (stick || buttonChanged) {

        JoystickEvent *event = new JoystickEvent();

        for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
          event->SetAxis(j, i, axes[i]);
        }

        // needs windowing event?
        Vector3 stick1Direction = Vector3(axes[0], axes[1], 0);
        if (j == GetActiveJoystickID() && stick1Direction.GetLength() != 0) {
          wEvent->SetDirection(stick1Direction);
          needsWindowingEvent = true;
        }

        for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
          bool pressed = UserEventManager::GetInstance().GetJoyButtonState(j, i);
          if (pressed && !prevButtonState[j][i]) {
            event->SetButton(j, i);
            if (j == GetActiveJoystickID() && i == joyButtonActivate) {
              wEvent->SetActivate();
              needsWindowingEvent = true;
            }
            if (j == GetActiveJoystickID() && i == joyButtonEscape) {
              wEvent->SetEscape();
              needsWindowingEvent = true;
            }
          }
          prevButtonState[j][i] = pressed;
        }

        if (windowManager->GetFocus() == currentFocus) windowManager->GetFocus()->ProcessEvent(event);
        delete event;
      }
    }

    if (needsWindowingEvent)
      if (windowManager->GetFocus() == currentFocus) windowManager->GetFocus()->ProcessEvent(wEvent);

    delete wEvent;

  }

  void Gui2Task::SetEventJoyButtons(int activate, int escape) {
    this->joyButtonActivate = activate;
    this->joyButtonEscape = escape;
  }

}
