// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "replaymenu.hpp"

#include "main.hpp"
#include "framework/scheduler.hpp"

#include "../../hid/keyboard.hpp"

#include "managers/environmentmanager.hpp"

using namespace blunted;

ReplayPage::ReplayPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  match = GetGameTask()->GetMatch();

  this->SetFocus();
  this->Show();

  signed long tmp = match->GetActualTime_ms() - match->GetReplaySize_ms(); // must be signed for negative numbers
  minTime_ms = std::max((signed long)10, tmp);
  maxTime_ms = std::max((long unsigned int)10, match->GetActualTime_ms() - 10);
  actualTime_ms = clamp(maxTime_ms - 3000, minTime_ms, maxTime_ms);
  replayCamCount = match->GetReplayCamCount();

  cam = 0;
  modifierValue = 0.0f;
  autoRun = false;
  stayInReplay = true;

  sig_OnClose.connect(boost::bind(&ReplayPage::OnClose, this));

  match->SetAutoUpdateIngameCamera(false);

  match->replayState.Lock();
  match->replayState->viewTime_ms = actualTime_ms;//minTime_ms;
  match->replayState->cam = cam;
  match->replayState->modifierValue = 0.0f;
  match->replayState->dirty = true;
  match->replayState.Unlock();
}

ReplayPage::~ReplayPage() {
}

void ReplayPage::OnClose() {

  match->replayState.Lock();
  match->replayState->viewTime_ms = maxTime_ms;
  match->replayState->cam = cam;
  match->replayState->modifierValue = 0.0f;
  match->replayState->dirty = true;
  match->replayState.Unlock();

  GetScheduler()->ResetTaskSequenceTime("game");
  match->SetAutoUpdateIngameCamera(true);

  if (stayInReplay) match->Pause(false); // todo: handle gracefully instead of using stayInReplay :p only unpause when started from gamepage instead of ingame page
}

void ReplayPage::Autorun(int replayHistoryOffset_ms, bool stayInReplay) {
  autoRun = true;
  cam = 1;
  modifierValue = 0.0;
  signed long tmp = maxTime_ms - replayHistoryOffset_ms;
  actualTime_ms = clamp(tmp, minTime_ms, maxTime_ms);
  this->stayInReplay = stayInReplay;
}

void ReplayPage::Process() {
  if (autoRun) {
    Vector3 direction;
    direction.coords[0] = 0.5f;
    ProcessInput(direction, false, false);
  }
}

void ReplayPage::ProcessKeyboardEvent(KeyboardEvent *event) {

  const std::vector<IHIDevice*> &controllers = GetControllers();
  HIDKeyboard *keyboard = static_cast<HIDKeyboard*>(controllers.at(0));

  bool button1 = false;
  bool button2 = false;
  if (event->GetKeyOnce(keyboard->GetFunctionMapping(e_ButtonFunction_ShortPass))) button1 = true;
  if (event->GetKeyOnce(keyboard->GetFunctionMapping(e_ButtonFunction_HighPass)))  button2 = true;

  Vector3 direction;
  if (event->GetKeyContinuous(keyboard->GetFunctionMapping(e_ButtonFunction_Left)))  direction.coords[0] += -0.5f;
  if (event->GetKeyContinuous(keyboard->GetFunctionMapping(e_ButtonFunction_Right))) direction.coords[0] +=  0.5f;
  if (event->GetKeyContinuous(keyboard->GetFunctionMapping(e_ButtonFunction_Up)))    direction.coords[1] += -0.5f;
  if (event->GetKeyContinuous(keyboard->GetFunctionMapping(e_ButtonFunction_Down)))  direction.coords[1] +=  0.5f;

  ProcessInput(direction, button1, button2);

}

void ReplayPage::ProcessJoystickEvent(JoystickEvent *event) {

  int controllerID = 0;
  const std::vector<IHIDevice*> &controllers = GetControllers();
  HIDGamepad *gamepad = static_cast<HIDGamepad*>(controllers.at(controllerID + 1)); // todo: check if we can be sure this is actually a joystick/gamepad
  bool button1 = event->GetButton(0, gamepad->GetControllerMapping(gamepad->GetFunctionMapping(e_ButtonFunction_LongPass))) ||
                 event->GetButton(0, gamepad->GetControllerMapping(gamepad->GetFunctionMapping(e_ButtonFunction_ShortPass))); // need 2 options because maybe the first is set to gui's 'escape' function
  bool button2 = event->GetButton(0, gamepad->GetControllerMapping(gamepad->GetFunctionMapping(e_ButtonFunction_HighPass))) ||
                 event->GetButton(0, gamepad->GetControllerMapping(gamepad->GetFunctionMapping(e_ButtonFunction_Shot))); // need 2 options because maybe the first is set to gui's 'escape' function

  Vector3 direction;
  direction.coords[0] = event->GetAxis(0, 0);
  direction.coords[1] = event->GetAxis(0, 1);

  float deadzone = 0.2f;
  if (fabs(direction.coords[0]) < deadzone) {
    direction.coords[0] = 0.0f;
  } else {
    direction.coords[0] = pow((fabs(direction.coords[0]) - deadzone) * (1.0f / (1.0f - deadzone)), 2.0f) * signSide(direction.coords[0]);
  }
  deadzone = 0.4f;
  if (fabs(direction.coords[1]) < deadzone) {
    direction.coords[1] = 0.0f;
  } else {
    direction.coords[1] = pow((fabs(direction.coords[1]) - deadzone) * (1.0f / (1.0f - deadzone)), 4.0f) * signSide(direction.coords[1]);
  }

  ProcessInput(direction, button1, button2);

}

void ReplayPage::ProcessInput(const Vector3 &direction, bool button1, bool button2) {

  // autorun
  if (button2 && autoRun == false) {
    actualTime_ms = minTime_ms;
    autoRun = true;
  } else if (button2) {
    autoRun = false;
  }
  if (button1 && autoRun == true) {
    autoRun = false;
  } else if (button1) {
    cam++;
    if (cam == replayCamCount) cam = 0;
  }

  if (!autoRun) {
    modifierValue += direction.coords[1] * 0.05f;
  }

  if (cam == 2) {
    if (modifierValue < -1.0f) modifierValue += 2.0f;
    if (modifierValue >  1.0f) modifierValue -= 2.0f;
  } else {
    modifierValue = clamp(modifierValue, -1.0f, 1.0f);
  }

  float timeMovement = direction.coords[0] * 2.0f;
  actualTime_ms += int(round(timeMovement * 10.0f));

  if (autoRun && actualTime_ms >= (signed int)maxTime_ms) {
    autoRun = false;
  }

  actualTime_ms = clamp(actualTime_ms, minTime_ms, maxTime_ms);
  //printf("min time: %lu, max time: %lu\n", minTime_ms, maxTime_ms);
  //unsigned long resultTime = clamp(actualTime_ms + PredictFrameTimeToGo_ms(7) * int(round(timeMovement)), minTime_ms, maxTime_ms);
  unsigned long resultTime = actualTime_ms;
  //printf("%lu\n", resultTime);


  // feed results to match - replays are effectively replayed there

  match->replayState.Lock();
  match->replayState->viewTime_ms = resultTime;
  match->replayState->cam = cam;
  match->replayState->modifierValue = modifierValue;
  match->replayState->dirty = true;
  match->replayState.Unlock();

}
