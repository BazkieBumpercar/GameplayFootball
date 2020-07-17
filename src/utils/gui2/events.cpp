// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "events.hpp"

namespace blunted {

  Gui2Event::Gui2Event(e_Gui2EventType eventType) : eventType(eventType), accepted(false) {
  }

  Gui2Event::~Gui2Event() {
  }

  e_Gui2EventType Gui2Event::GetType() const {
    return eventType;
  }


  // WINDOWING EVENT

  WindowingEvent::WindowingEvent() : Gui2Event(e_Gui2EventType_Windowing) {
    activate = false;
    escape = false;
  }

  WindowingEvent::~WindowingEvent() {
  }


  // KEYBOARD EVENT

  KeyboardEvent::KeyboardEvent() : Gui2Event(e_Gui2EventType_Keyboard) {
    for (int i = 0; i < SDLK_LAST; i++) {
      keyOnce[i] = false;
      keyContinuous[i] = false;
      keyRepeated[i] = false;
    }
  }

  KeyboardEvent::~KeyboardEvent() {
  }


  // JOYSTICK EVENT

  JoystickEvent::JoystickEvent() : Gui2Event(e_Gui2EventType_Joystick) {
    for (int j = 0; j < UserEventManager::GetInstance().GetJoystickCount(); j++) {
      for (int i = 0; i < _JOYSTICK_MAXBUTTONS; i++) {
        button[j][i] = false;
      }
    }
  }

  JoystickEvent::~JoystickEvent() {
  }

}
