// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MATCHDATA
#define _HPP_MATCHDATA

#include "defines.hpp"

#include "../gamedefines.hpp"

#include "teamdata.hpp"

class MatchData {

  public:
    MatchData(int team1DatabaseID, int team2DatabaseID);
    virtual ~MatchData();

    TeamData *GetTeamData(int id) { return teamData[id]; }
    int GetGoalCount(int id) { return goalCount[id]; }
    void SetGoalCount(int id, int amount) { goalCount[id] = amount; }
    void AddPossessionTime_10ms(int teamID);
    unsigned long GetPossessionTime_ms(int teamID) { return possessionTime_ms[teamID]; }
    float GetPossessionFactor_60seconds() { return possession60seconds / 60.0f * 0.5f + 0.5f; } // REMEMBER THESE ARE IRL INGAME SECONDS (because, I guess the tactics should be based on irl possession time instead of gametime? not sure yet, think about this)
    void AddShot(int teamID) { shots[teamID] += 1; }
    int GetShots(int teamID) { return shots[teamID]; }

  protected:
    TeamData *teamData[2];

    int goalCount[2];

    unsigned long possessionTime_ms[2];
    float possession60seconds; // -600 to 600 for possession of team 1 / 2 respectively
    int shots[2];

};

#endif
