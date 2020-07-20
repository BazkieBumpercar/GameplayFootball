// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "keyboard.hpp"

#include "managers/usereventmanager.hpp"

#include "../main.hpp"

HIDKeyboard::HIDKeyboard() {

  deviceType = e_HIDeviceType_Keyboard;
  identifier = "keyboard";

  LoadConfig();
}

HIDKeyboard::~HIDKeyboard() {
}

void HIDKeyboard::LoadConfig() {
  boost::mutex::scoped_lock blah(mutex);
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    functionButtonState[i] = false;
    previousFunctionButtonState[i] = false;

    functionMapping[i] = (SDL_Keycode)GetConfiguration()->GetInt(("input_keyboard_" + int_to_str(i)).c_str(), (SDL_Keycode)defaultKeyIDs[i]);
  }
}

void HIDKeyboard::SaveConfig() {
  boost::mutex::scoped_lock blah(mutex);
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    GetConfiguration()->SetInt(("input_keyboard_" + int_to_str(i)).c_str(), functionMapping[i]);
  }
  GetConfiguration()->SaveFile(GetConfigFilename());
}

void HIDKeyboard::Process() {
  boost::mutex::scoped_lock blah(mutex);
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    previousFunctionButtonState[i] = functionButtonState[i];
    functionButtonState[i] = UserEventManager::GetInstance().GetKeyboardState(functionMapping[i]);
  }
}

bool HIDKeyboard::GetButton(e_ButtonFunction buttonFunction) {
  boost::mutex::scoped_lock blah(mutex);
  return functionButtonState[buttonFunction];
}

float HIDKeyboard::GetButtonValue(e_ButtonFunction buttonFunction) {
  boost::mutex::scoped_lock blah(mutex);
  if (functionButtonState[buttonFunction]) return 1.0; else return 0.0;
}

void HIDKeyboard::SetButton(e_ButtonFunction buttonFunction, bool state) {
  boost::mutex::scoped_lock blah(mutex);
  functionButtonState[buttonFunction] = state;
}

bool HIDKeyboard::GetPreviousButtonState(e_ButtonFunction buttonFunction) {
  boost::mutex::scoped_lock blah(mutex);
  return previousFunctionButtonState[buttonFunction];
}

Vector3 HIDKeyboard::GetDirection() {
  Vector3 inputDirection;
  if (GetButton(e_ButtonFunction_Left))  inputDirection.coords[0] -= 1;
  if (GetButton(e_ButtonFunction_Right)) inputDirection.coords[0] += 1;
  if (GetButton(e_ButtonFunction_Up))    inputDirection.coords[1] += 1;
  if (GetButton(e_ButtonFunction_Down))  inputDirection.coords[1] -= 1;
  inputDirection.Normalize(0);
  return inputDirection;
}
