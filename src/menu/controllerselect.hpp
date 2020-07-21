// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_CONTROLLERSELECT
#define _HPP_MENU_CONTROLLERSELECT

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/image.hpp"

#include "../onthepitch/match.hpp"

using namespace blunted;

class ControllerSelectPage : public Gui2Page {

  public:
    ControllerSelectPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~ControllerSelectPage();

    void SetImagePositions();

    virtual void Process();
    virtual void ProcessKeyboardEvent(KeyboardEvent *event);
    virtual void ProcessJoystickEvent(JoystickEvent *event);
    virtual void ProcessWindowingEvent(WindowingEvent *event);

  protected:
    std::vector<SideSelection> sides;
    std::vector<unsigned long> delay;
    bool inGame;

};

#endif
