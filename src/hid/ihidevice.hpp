// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_HIDEVICE
#define _HPP_HIDEVICE

#include "base/math/vector3.hpp"

using namespace blunted;

enum e_HIDeviceType {
  e_HIDeviceType_Keyboard,
  e_HIDeviceType_Gamepad
};

enum e_ButtonFunction {
  e_ButtonFunction_Up,
  e_ButtonFunction_Right,
  e_ButtonFunction_Down,
  e_ButtonFunction_Left,
  e_ButtonFunction_LongPass,
  e_ButtonFunction_HighPass,
  e_ButtonFunction_ShortPass,
  e_ButtonFunction_Shot,
  e_ButtonFunction_KeeperRush,
  e_ButtonFunction_Sliding,
  e_ButtonFunction_Pressure,
  e_ButtonFunction_TeamPressure,
  e_ButtonFunction_Switch,
  e_ButtonFunction_Special,
  e_ButtonFunction_Sprint,
  e_ButtonFunction_Dribble,
  e_ButtonFunction_Select,
  e_ButtonFunction_Start,
  e_ButtonFunction_Size
};

enum e_ControllerButton {
  e_ControllerButton_Up,
  e_ControllerButton_Right,
  e_ControllerButton_Down,
  e_ControllerButton_Left,
  e_ControllerButton_Y,
  e_ControllerButton_B,
  e_ControllerButton_A,
  e_ControllerButton_X,
  e_ControllerButton_L1,
  e_ControllerButton_L2,
  e_ControllerButton_R1,
  e_ControllerButton_R2,
  e_ControllerButton_Select,
  e_ControllerButton_Start,
  e_ControllerButton_Size
};

class IHIDevice {

  public:
    virtual ~IHIDevice() {}

    virtual void LoadConfig() = 0;
    virtual void SaveConfig() = 0;

    virtual void Process() = 0;

    virtual bool GetButton(e_ButtonFunction buttonFunction) = 0;
    virtual float GetButtonValue(e_ButtonFunction buttonFunction) = 0; // for analog support
    virtual void SetButton(e_ButtonFunction buttonFunction, bool state) = 0;
    virtual bool GetPreviousButtonState(e_ButtonFunction buttonFunction) = 0;
    virtual Vector3 GetDirection() = 0;

    e_HIDeviceType GetDeviceType() const { return deviceType; }
    std::string GetIdentifier() const { return identifier; }

  protected:
    e_HIDeviceType deviceType;
    std::string identifier;

    boost::mutex mutex;

};

#endif
