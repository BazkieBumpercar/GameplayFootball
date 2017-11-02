// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TEAMDATA
#define _HPP_TEAMDATA

#include "defines.hpp"
#include "base/properties.hpp"

#include "../gamedefines.hpp"
#include "playerdata.hpp"

struct TeamTactics {

  TeamTactics() {
  }

  Properties factoryProperties;
  Properties userProperties;

  Properties humanReadableNames;
  Properties descriptions;

};

class TeamData {

  public:
    TeamData(int teamDatabaseID);
    virtual ~TeamData();

    std::string GetName() { return name; }
    std::string GetShortName() { return shortName; }
    std::string GetLogoUrl() { return logo_url; }
    std::string GetKitUrl() { return kit_url; }
    Vector3 GetColor1() { return color1; }
    Vector3 GetColor2() { return color2; }

    int GetDatabaseID() const { return databaseID; }

    const TeamTactics &GetTactics() const { return tactics; }
    TeamTactics &GetTacticsWritable() { return tactics; }

    FormationEntry GetFormationEntry(int num);
    void SetFormationEntry(int num, FormationEntry entry);

    void SwitchPlayers(int databaseID1, int databaseID2);

    // vector index# is entry in formation[index#]
    const std::vector<PlayerData*> &GetPlayerData() { return playerData; }
    int GetPlayerNum() { return playerData.size(); }
    PlayerData *GetPlayerData(int num) { return playerData.at(num); }
    PlayerData *GetPlayerDataByDatabaseID(int id);

    void SaveLineup();
    void SaveTactics();
    void Save();

  protected:
    int databaseID;

    std::string name;
    std::string shortName;
    std::string logo_url;
    std::string kit_url;
    Vector3 color1, color2;

    TeamTactics tactics;

    FormationEntry formation[playerNum];

    std::vector<PlayerData*> playerData;

};

#endif
