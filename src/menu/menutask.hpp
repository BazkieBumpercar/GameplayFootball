// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_MENUTASK
#define _HPP_GUI2_MENUTASK

#include "ingame/gamepage.hpp"

#include "utils/gui2/guitask.hpp"

#include "utils/gui2/widgets/image.hpp"

#include "scene/scene3d/scene3d.hpp"

#include "../gamedefines.hpp"

class Match;
class MatchData;

using namespace blunted;

enum e_MenuAction {
  e_MenuAction_Menu, // start main menu
  e_MenuAction_Game, // start game
  e_MenuAction_None
};

struct SideSelection {
  int controllerID;
  Gui2Image *controllerImage;
  int side; // -1, 0, 1
};

// todo: just load match-, team-, and playerdata before starting match
// this requires some bigger changes, so stick with this imperfect system for the time being
struct QueuedFixture {
  QueuedFixture() {
    team1KitNum = 1;
    team2KitNum = 2;
    matchData = 0;
  }
  std::vector<SideSelection> sides; // queued match fixture
  std::string teamID1, teamID2; // queued match fixture
  int team1KitNum, team2KitNum;
  MatchData *matchData;
};

void SetActiveController(int side, bool keyboard);

class MenuTask : public Gui2Task {

  public:
    MenuTask(float aspectRatio, float margin, TTF_Font *defaultFont, TTF_Font *defaultOutlineFont);
    virtual ~MenuTask();

    virtual void ProcessPhase();

    bool QuickStart();
    void QuitGame();

    void ReleaseAllButtons();

    void SetControllerSetup(const std::vector<SideSelection> &sides) { queuedFixture.Lock(); queuedFixture->sides = sides; queuedFixture.Unlock(); }
    const std::vector<SideSelection> GetControllerSetup() { return queuedFixture.GetData().sides; }
    void SetTeamIDs(const std::string &id1, const std::string &id2) { queuedFixture.Lock(); queuedFixture->teamID1 = id1; queuedFixture->teamID2 = id2; queuedFixture.Unlock(); }
    int GetTeamID(int whichOne) { if (whichOne == 0) return atoi(queuedFixture.GetData().teamID1.c_str()); else return atoi(queuedFixture.GetData().teamID2.c_str()); }
    int GetTeamKitNum(int teamID) { if (teamID == 0) return queuedFixture.GetData().team1KitNum; else return queuedFixture.GetData().team2KitNum; }
    void SetMatchData(MatchData *matchData) { queuedFixture.Lock(); queuedFixture->matchData = matchData; queuedFixture.Unlock(); }
    MatchData *GetMatchData() { return queuedFixture.GetData().matchData; } // hint: this lock is useless, since we are returning the pointer and not a copy

    void SetMenuAction(e_MenuAction menuAction) { this->menuAction = menuAction; }

  protected:
    e_MenuAction menuAction;

    Lockable<QueuedFixture> queuedFixture; // todo: we can probably unlock this stuff

};

#endif
