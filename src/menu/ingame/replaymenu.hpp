// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_INGAME_REPLAY
#define _HPP_MENU_INGAME_REPLAY

#include "utils/gui2/windowmanager.hpp"
#include "utils/gui2/page.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

class ReplayPage : public Gui2Page {

  public:
    ReplayPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~ReplayPage();

    void OnClose();
    void Autorun(int replayHistoryOffset_ms, bool stayInReplay);

  protected:

    Match *match;

    virtual void Process();
    virtual void ProcessKeyboardEvent(KeyboardEvent *event);
    virtual void ProcessJoystickEvent(JoystickEvent *event);
    void ProcessInput(const Vector3 &direction, bool button1, bool button2);

    signed long actualTime_ms;
    unsigned long minTime_ms;
    unsigned long maxTime_ms;

    int cam;
    int replayCamCount;
    float modifierValue;

    bool autoRun;
    bool stayInReplay;

};

#endif
