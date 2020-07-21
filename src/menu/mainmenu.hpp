// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_MAIN
#define _HPP_MENU_MAIN

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/grid.hpp"
#include "utils/gui2/widgets/button.hpp"
#include "utils/gui2/widgets/image.hpp"
#include "utils/gui2/widgets/capturekey.hpp"

#include "../utils.hpp"

#include "../onthepitch/match.hpp"

using namespace blunted;


// import structs

struct CountryCompetition {
  std::string country;
  std::string competition;
};

struct PlayerImport;

struct Club {
  int id;
  std::string name;
  std::string country;
  std::string competition;
  int reputation;
  signed int balance;
  std::string status;

  std::vector<PlayerImport> players;
};

// non-importables
struct ClubData {
  std::string formation_xml;
  std::string tactics_xml;
  std::string shortName;
  std::string color1;
  std::string color2;
};

struct PlayerImport {
  int id;
  std::string firstName;
  std::string lastName;
  std::string nationality;
  std::string position;
  int age;
  int value;
  int clubID;
  std::vector<Stat> profileStats;
  float averageStat; // for sorting who is in starting 11, not used otherwise
  float averageStartStat; // calculated back to 15 year old
};


class IntroPage : public Gui2Page {

  public:
    IntroPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~IntroPage();

    virtual void ProcessWindowingEvent(WindowingEvent *event);
    virtual void ProcessKeyboardEvent(KeyboardEvent *event);

  protected:
    Gui2Image *bg;

};

class OutroPage : public Gui2Page {

  public:
    OutroPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~OutroPage();

    virtual void ProcessWindowingEvent(WindowingEvent *event);
    virtual void ProcessKeyboardEvent(KeyboardEvent *event);

  protected:
    Gui2Image *bg;

};

class MainMenuPage : public Gui2Page {

  public:
    MainMenuPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~MainMenuPage();

    void GoControllerSelect();
    void GoLeague();
    void GoSettings();
    void GoCredits();
    void GoOutro();
    bool GoImportDB();
    //XX void Import_AgeValueStatsAdd(int age, int value);
    //XX void Import_ProcessAgeValueStats();

    virtual void ProcessWindowingEvent(WindowingEvent *event);

  protected:
    Gui2Grid *grid;

    std::vector<Gui2Button*> buttons;

    std::map < int, std::vector<int> > ageValues;
    std::map < int, float > averageStatPerAge;

};

#endif
