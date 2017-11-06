// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_LEAGUE_TEAM
#define _HPP_MENU_LEAGUE_TEAM

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

class LeagueTeamPage : public Gui2Page {

  public:
    LeagueTeamPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamPage();

  protected:

};

class LeagueTeamFormationPage : public Gui2Page {

  public:
    LeagueTeamFormationPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamFormationPage();

  protected:

};

class LeagueTeamPlayerSelectionPage : public Gui2Page {

  public:
    LeagueTeamPlayerSelectionPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamPlayerSelectionPage();

  protected:

};

class LeagueTeamTacticsPage : public Gui2Page {

  public:
    LeagueTeamTacticsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamTacticsPage();

  protected:

};

class LeagueTeamPlayerOverviewPage : public Gui2Page {

  public:
    LeagueTeamPlayerOverviewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamPlayerOverviewPage();

  protected:

};

class LeagueTeamPlayerDevelopmentPage : public Gui2Page {

  public:
    LeagueTeamPlayerDevelopmentPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamPlayerDevelopmentPage();

  protected:

};

class LeagueTeamSetupPage : public Gui2Page {

  public:
    LeagueTeamSetupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueTeamSetupPage();

  protected:

};

#endif
