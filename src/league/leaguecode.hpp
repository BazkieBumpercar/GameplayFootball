// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LEAGUECODE
#define _HPP_LEAGUECODE

#include "../main.hpp"

using namespace blunted;

int CreateNewLeagueSave(const std::string &srcDbName, const std::string &saveName);
bool PrepareDatabaseForLeague();
bool SaveAutosaveToDatabase();
bool SaveDatabaseToAutosave();
bool LoadLeague();
void GenerateSeasonCalendars();

#endif
