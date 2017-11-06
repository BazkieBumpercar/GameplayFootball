// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_GAMEPLAN_SUBMENU
#define _HPP_MENU_GAMEPLAN_SUBMENU

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/slider.hpp"

using namespace blunted;

class GamePlanSubMenu : public Gui2View {

  public:
    GamePlanSubMenu(Gui2WindowManager *windowManager, Gui2View *parentFocus, Gui2Grid *mainGrid, const std::string &name);
    virtual ~GamePlanSubMenu();

    void OnClose();

    Gui2Button *AddButton(const std::string &buttonName, const std::string &buttonCaption, int row, int column, Vector3 color);
    Gui2Slider *AddSlider(const std::string &sliderName, const std::string &sliderCaption, int row, int column);

    // returns first toggled button in grid
    Gui2Button *GetToggledButton(Gui2Button *except);

    Gui2Grid *GetGrid() { return grid; }

    const std::vector<Gui2Button*> &GetAllButtons() { return allButtons; }

    void ProcessWindowingEvent(WindowingEvent *event);

  protected:
    Gui2Grid *grid;
    Gui2Grid *mainGrid; // root menu's grid
    Gui2View *parentFocus;

    std::vector<Gui2Button*> allButtons; // cache for GetToggledbutton()

};

#endif
