// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_HIDKEYBOARD
#define _HPP_HIDKEYBOARD

#include "base/math/vector3.hpp"

#include <SDL/SDL.h>

#include "ihidevice.hpp"

using namespace blunted;

class HIDKeyboard : public IHIDevice {

  public:
    HIDKeyboard();
    virtual ~HIDKeyboard();

    virtual void LoadConfig();
    virtual void SaveConfig();

    virtual void Process();

    virtual bool GetButton(e_ButtonFunction buttonFunction);
    virtual float GetButtonValue(e_ButtonFunction buttonFunction); // for analog support
    virtual void SetButton(e_ButtonFunction buttonFunction, bool state);
    virtual bool GetPreviousButtonState(e_ButtonFunction buttonFunction);
    virtual Vector3 GetDirection();

    void SetFunctionMapping(int index, SDLKey key) { boost::mutex::scoped_lock blah(mutex); functionMapping[index] = key; }

    SDLKey GetFunctionMapping(e_ButtonFunction buttonFunction) { boost::mutex::scoped_lock blah(mutex); return functionMapping[buttonFunction]; }

  protected:
    bool functionButtonState[e_ButtonFunction_Size];
    bool previousFunctionButtonState[e_ButtonFunction_Size];

    SDLKey functionMapping[e_ButtonFunction_Size];

};

#endif
