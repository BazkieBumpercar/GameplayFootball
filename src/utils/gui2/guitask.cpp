// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "guitask.hpp"

#include "managers/usereventmanager.hpp"

namespace blunted {

  Gui2Task::Gui2Task(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin) : scene2D(scene2D) {
    windowManager = new Gui2WindowManager(scene2D, aspectRatio, margin);
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

    auto currentKeyState = UserEventManager::GetInstance().GetKeyboardState();

    for (auto keyState : currentKeyState) {
      auto pressedKey = keyState.first;

      // continuous
      event->SetKeyContinuous(pressedKey);
      //printf("key %i pressed\n", pressedKey);

      needsKeyboardEvent = true;

      // only when pressed first time
      if (prevKeyState.count(pressedKey) == 0) {
        event->SetKeyOnce(pressedKey);

        if (keyboard) {
          if (pressedKey == SDLK_RETURN) {
            wEvent->SetActivate();
            needsWindowingEvent = true;
          }
          else if (pressedKey == SDLK_ESCAPE) {
            wEvent->SetEscape();
            needsWindowingEvent = true;
          }
        }
      }

      // when pressed first time, or when held for longer than some threshold
      if (prevKeyState.count(pressedKey) == 0 || UserEventManager::GetInstance().GetLastKeyPressDiff_ms(pressedKey) > 250) {
        event->SetKeyRepeated(pressedKey);

        if (keyboard) {
          if (pressedKey == SDLK_UP) {
            wEvent->SetDirection(Vector3(0, -1, 0));
            needsWindowingEvent = true;
          } else if (pressedKey == SDLK_RIGHT) {
            wEvent->SetDirection(Vector3(1, 0, 0));
            needsWindowingEvent = true;
          } else if (pressedKey == SDLK_DOWN) {
            wEvent->SetDirection(Vector3(0, 1, 0));
            needsWindowingEvent = true;
          } else if (pressedKey == SDLK_LEFT) {
            wEvent->SetDirection(Vector3(-1, 0, 0));
            needsWindowingEvent = true;
          }
        }
      }
    }

    prevKeyState = currentKeyState;

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
