// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_INGAME
#define _HPP_MENU_INGAME

#include "utils/gui2/windowmanager.hpp"

#include "../cameramenu.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

class IngamePage : public Gui2Page {

  public:
    IngamePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~IngamePage();

    void GoGamePlan();
    void GoControllerSelect();
    void GoCameraSettings();
    void GoVisualOptions();
    void GoSystemSettings();
    void GoReplay();
    void GoPreQuit();

    virtual void ProcessWindowingEvent(WindowingEvent *event);

  protected:
    int teamID; // team that activated the ingame menu

};


class PreQuitPage : public Gui2Page {

  public:
    PreQuitPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~PreQuitPage();

    void GoMenu();

  protected:

};

#endif
