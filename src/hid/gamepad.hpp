// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_HIDGAMEPAD
#define _HPP_HIDGAMEPAD

#include "base/math/vector3.hpp"

#include "ihidevice.hpp"

using namespace blunted;

class HIDGamepad : public IHIDevice {

  public:
    HIDGamepad(int gamepadID);
    virtual ~HIDGamepad();

    virtual void LoadConfig();
    virtual void SaveConfig();

    virtual void Process();

    virtual bool GetButton(e_ButtonFunction buttonFunction);
    virtual float GetButtonValue(e_ButtonFunction buttonFunction); // for analog support
    virtual void SetButton(e_ButtonFunction buttonFunction, bool state);
    virtual bool GetPreviousButtonState(e_ButtonFunction buttonFunction);
    virtual Vector3 GetDirection();

    e_ControllerButton GetFunctionMapping(e_ButtonFunction buttonFunction) { boost::mutex::scoped_lock blah(mutex); return functionMapping[buttonFunction]; }
    void SetFunctionMapping(e_ButtonFunction buttonFunction, e_ControllerButton controllerButton) { boost::mutex::scoped_lock blah(mutex); functionMapping[buttonFunction] = controllerButton; }
    signed int GetControllerMapping(e_ControllerButton controllerButton) { boost::mutex::scoped_lock blah(mutex); return controllerMapping[controllerButton]; }
    void SetControllerMapping(e_ControllerButton controllerButton, signed int id) { boost::mutex::scoped_lock blah(mutex); controllerMapping[controllerButton] = id; }

    int GetGamepadID() { return gamepadID; }

  protected:
    int gamepadID;
    float controllerButtonState[e_ControllerButton_Size];
    float previousControllerButtonState[e_ControllerButton_Size];

    e_ControllerButton functionMapping[e_ButtonFunction_Size];
    signed int controllerMapping[e_ControllerButton_Size];

};

#endif
