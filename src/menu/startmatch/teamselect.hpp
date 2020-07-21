// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_TEAMSELECT
#define _HPP_MENU_TEAMSELECT

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/image.hpp"
#include "utils/gui2/widgets/slider.hpp"
#include "utils/gui2/widgets/iconselector.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

class TeamSelectPage : public Gui2Page {

  public:
    TeamSelectPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~TeamSelectPage();

    void FocusTeamSelect1();
    void FocusStart1();
    void FocusCompetitionSelect2();
    void FocusTeamSelect2();
    void FocusStart2();
    void SetupTeamSelect1();
    void SetupTeamSelect2();
    void GoOptionsMenu();

    virtual void ProcessWindowingEvent(WindowingEvent *event);

    Gui2Button *buttonStart1;
    Gui2Button *buttonStart2;
    Gui2Caption *p2;
    Gui2Grid *grid2;
    Gui2Image *bg2;

  protected:
    Gui2IconSelector *teamSelect1;
    Gui2IconSelector *teamSelect2;
    Gui2IconSelector *competitionSelect1;
    Gui2IconSelector *competitionSelect2;

};

#endif
