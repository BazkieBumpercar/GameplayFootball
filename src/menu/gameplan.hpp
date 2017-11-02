// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_GAMEPLAN
#define _HPP_MENU_GAMEPLAN

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/page.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/slider.hpp"
#include "utils/gui2/widgets/image.hpp"

#include "widgets/gameplansubmenu.hpp"

#include "widgets/planmap.hpp"

#include "../onthepitch/match.hpp"

#include "../data/teamdata.hpp"

#include "utils/database.hpp"

using namespace blunted;

struct TacticsSlider {
  int id;
  Gui2Slider* widget;
  std::string tacticName;
};

class GamePlanPage : public Gui2Page {

  public:
    GamePlanPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamePlanPage();

    void OnClose();

    virtual void Deactivate();
    virtual void Reactivate();

    Vector3 GetButtonColor(int id);

    void GoLineupMenu();
    void LineupMenuOnClick(Gui2Button *button);
    void SaveLineup();

    void GoTacticsMenu();
    void TacticsMenuOnChange(Gui2Slider *slider, int id);
    void SaveTactics();

  protected:
    int teamID;

    Gui2PlanMap *map;
    Gui2Grid *grid;
    Gui2Grid *gridNav;
    Gui2Button *buttonLineup;
    Gui2Button *buttonTactics;

    TeamData *teamData;

    GamePlanSubMenu *lineupMenu;
    GamePlanSubMenu *tacticsMenu;

    std::vector<TacticsSlider> tacticsSliders;

    Database *namedb;

};

#endif
