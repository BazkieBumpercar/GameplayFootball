// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_LEAGUE
#define _HPP_MENU_LEAGUE

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/page.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/editline.hpp"
#include "utils/gui2/widgets/dialog.hpp"
#include "utils/gui2/widgets/filebrowser.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/pulldown.hpp"
#include "utils/gui2/widgets/slider.hpp"

using namespace blunted;

class LeaguePage : public Gui2Page {

  public:
    LeaguePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeaguePage();

  protected:
    void StepTime();
    void SetTimeCaption();

    Gui2Caption *captionTime;

};

class LeagueStartPage : public Gui2Page {

  public:
    LeagueStartPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartPage();

  protected:
    void GoLoad();
    void GoNew();

};

class LeagueStartLoadPage : public Gui2Page {

  public:
    LeagueStartLoadPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartLoadPage();

  protected:
    void GoLoadSave();

    Gui2FileBrowser *browser;

};

class LeagueStartNewPage : public Gui2Page {

  public:
    LeagueStartNewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~LeagueStartNewPage();

  protected:
    void GoDatabaseSelectDialog();
    void CloseDatabaseSelectDialog();
    void GoProceed();
    void CloseCreateSaveDialog();

    Gui2Button *databaseSelectButton;
    Gui2Pulldown *currencySelectPulldown;
    Gui2Slider *difficultySlider;
    Gui2EditLine *saveNameInput;
    Gui2EditLine *managerNameInput;

    Gui2Dialog *databaseSelectDialog;
    Gui2FileBrowser *databaseSelectBrowser;
    Gui2View *previousFocus;

    std::string data_SelectedDatabase;

    Gui2Dialog *createSaveDialog;

    bool success;

};

#endif
