// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_LEAGUE_CALENDAR
#define _HPP_MENU_LEAGUE_CALENDAR

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/page.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/image.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/slider.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/caption.hpp"
#include "utils/gui2/widgets/capturekey.hpp"

using namespace blunted;

class LeagueCalendarPage : public Gui2Page {

  public:
    LeagueCalendarPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueCalendarPage();

  protected:

};

#endif
