// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_GAMEOVER
#define _HPP_MENU_GAMEOVER

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/page.hpp"

#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

class GameOverPage : public Gui2Page {

  public:
    GameOverPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GameOverPage();

    void GoRematch();
    void GoMainMenu();

    Gui2Button *buttonOkay;

  protected:
    Match *match;

};

#endif
