// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "matchdata.hpp"

MatchData::MatchData(int team1DatabaseID, int team2DatabaseID) {
  teamData[0] = new TeamData(team1DatabaseID);
  teamData[1] = new TeamData(team2DatabaseID);

  goalCount[0] = 0;
  goalCount[1] = 0;

  possessionTime_ms[0] = 0;
  possessionTime_ms[1] = 0;

  shots[0] = 0;
  shots[1] = 0;

  possession60seconds = 0.0f;
}

MatchData::~MatchData() {
  delete teamData[0];
  delete teamData[1];
}

void MatchData::AddPossessionTime_10ms(int teamID) {
  possessionTime_ms[teamID] += 10;
  if (teamID == 0) possession60seconds = std::max(possession60seconds - 0.01f, -60.0f);
  else if (teamID == 1) possession60seconds = std::min(possession60seconds + 0.01f, 60.0f);
  //printf("pos60s: %f\n", possession60seconds);
}
