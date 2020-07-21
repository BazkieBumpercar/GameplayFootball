// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "teamdata.hpp"

#include "utils/database.hpp"

#include "base/utils.hpp"

#include "../main.hpp"

#include <boost/algorithm/string.hpp>

Vector3 GetDefaultRolePosition(e_PlayerRole role) {
  switch (role) {

    case e_PlayerRole_GK:
      return Vector3(-1.0,  0.0, 0);
      break;

    case e_PlayerRole_CB:
      return Vector3(-1.0,  0.0, 0);
      break;
    case e_PlayerRole_LB:
      return Vector3(-0.8,  0.8, 0);
      break;
    case e_PlayerRole_RB:
      return Vector3(-0.8, -0.8, 0);
      break;

    case e_PlayerRole_DM:
      return Vector3(-0.5,  0.0, 0);
      break;
    case e_PlayerRole_CM:
      return Vector3( 0.0,  0.0, 0);
      break;
    case e_PlayerRole_LM:
      return Vector3( 0.0,  1.0, 0);
      break;
    case e_PlayerRole_RM:
      return Vector3( 0.0, -1.0, 0);
      break;
    case e_PlayerRole_AM:
      return Vector3( 0.5,  0.0, 0);
      break;

    case e_PlayerRole_CF:
      return Vector3( 1.0,  0.0, 0);
      break;

    default:
      return Vector3( 0.0,  0.0, 0);
      break;

  }
}

TeamData::TeamData(int teamDatabaseID) : databaseID(teamDatabaseID) {

  DatabaseResult *result = GetDB()->Query("select teams.name, teams.logo_url, teams.kit_url, teams.formation_xml, teams.formation_factory_xml, teams.tactics_xml, teams.tactics_factory_xml, teams.shortname, teams.color1, teams.color2 from teams, leagues where teams.id = " + int_to_str(databaseID) + " and leagues.id = teams.league_id limit 1");

  std::string formationString;
  std::string factoryFormationString;
  std::string factoryTacticsString;
  std::string tacticsString;

  color1.Set(0, 0, 0);
  color2.Set(255, 255, 255);

  bool national = false;

  for (unsigned int c = 0; c < result->data.at(0).size(); c++) {
    if (result->header.at(c).compare("national") == 0) national = (atoi(result->data.at(0).at(c).c_str()) == 0) ? false : true;

    if (result->header.at(c).compare("name") == 0) name = result->data.at(0).at(c);
    if (result->header.at(c).compare("logo_url") == 0) logo_url = result->data.at(0).at(c);
    if (result->header.at(c).compare("kit_url") == 0) kit_url = result->data.at(0).at(c);
    if (result->header.at(c).compare("formation_xml") == 0) formationString = result->data.at(0).at(c);
    if (result->header.at(c).compare("formation_factory_xml") == 0) factoryFormationString = result->data.at(0).at(c);
    if (result->header.at(c).compare("tactics_xml") == 0) tacticsString = result->data.at(0).at(c);
    if (result->header.at(c).compare("tactics_factory_xml") == 0) factoryTacticsString = result->data.at(0).at(c);
    if (result->header.at(c).compare("shortname") == 0) shortName = result->data.at(0).at(c);
    if (result->header.at(c).compare("color1") == 0) color1 = GetVectorFromString(result->data.at(0).at(c));
    if (result->header.at(c).compare("color2") == 0) color2 = GetVectorFromString(result->data.at(0).at(c));
  }

  if (shortName.compare("") == 0) {
    shortName = name;
    shortName.erase(remove_if(shortName.begin(), shortName.end(), isspace), shortName.end());
    shortName = boost::to_upper_copy(shortName.substr(0, 3));
  }

  delete result;

  logo_url = "databases/default/" + logo_url;
  kit_url = "databases/default/" + kit_url;


  // team formation

  XMLLoader loader;
  XMLTree tree = loader.Load(formationString);

  //loader.PrintTree(tree);

  map_XMLTree::const_iterator iter = tree.children.begin();
  while (iter != tree.children.end()) {
    for (int num = 0; num < playerNum; num++) {
      if ((*iter).first == "p" + int_to_str(num + 1)) {
        formation[num].databasePosition = GetVectorFromString((*iter).second.children.find("position")->second.value);
        formation[num].role = GetRoleFromString((*iter).second.children.find("role")->second.value);

        // combine custom positions with hardcoded formation positions belonging to certain roles.
        // this way, more extreme user formation settings are 'normalized' somewhat.
        formation[num].position = formation[num].databasePosition * 0.6f +
                                  GetDefaultRolePosition(formation[num].role) * 0.4f;

        //GetVectorFromString((*iter).second.children.find("position")->second.value).Print();
        //printf("%s\n", (*iter).second.children.find("role")->second.value.c_str());
      }
    }

    iter++;
  }

  // make sure players have some personal space, don't step in each other's aura ;)
  float minDistanceFraction = 0.5f; // remember the range is 2 (-1 to 1)
  unsigned int maxIterations = 10;
  unsigned int iterations = 0;
  bool changed = true;

  while (changed && iterations < maxIterations) {

    Vector3 offset[playerNum];

    changed = false;
    for (int p1 = 0; p1 < playerNum - 1; p1++) {
      if (formation[p1].role == e_PlayerRole_GK) continue;

      for (int p2 = p1 + 1; p2 < playerNum; p2++) {
        if (formation[p2].role == e_PlayerRole_GK) continue;

        Vector3 diff = (formation[p1].position - formation[p2].position);
        //if (diff.GetLength() < 0.1f) diff = Vector3(0, 0.1, 0);

        if (diff.GetLength() < minDistanceFraction) {
          changed = true;

          float distanceFactor = 1.0f - (diff.GetLength() / minDistanceFraction);

          offset[p1] += diff.GetNormalized(Vector3(0, 1, 0)) * minDistanceFraction * distanceFactor * 0.5f;
          offset[p2] -= diff.GetNormalized(Vector3(0, 1, 0)) * minDistanceFraction * distanceFactor * 0.5f;
        }
      }
    }

    if (changed) {
      for (int p = 0; p < playerNum; p++) {
        formation[p].position += offset[p];
        formation[p].position.coords[0] = clamp(formation[p].position.coords[0], -1, 1);
        formation[p].position.coords[1] = clamp(formation[p].position.coords[1], -1, 1);
      }
    }

    iterations++;
  }


  // team tactics

  tree = loader.Load(tacticsString);

  iter = tree.children.begin();
  while (iter != tree.children.end()) {
    tactics.userProperties.Set((*iter).first.c_str(), atof((*iter).second.value.c_str()));
    //printf("value name: %s, value: %f\n", (*iter).first.c_str(), atof((*iter).second.value.c_str()));

    if ((*iter).first.compare("position_offense_depth_factor") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "attacking: team depth");
      tactics.descriptions.Set((*iter).first.c_str(), "how much vertical space the team takes up, during possession");
    }
    else if ((*iter).first.compare("position_defense_depth_factor") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "defending: team depth");
      tactics.descriptions.Set((*iter).first.c_str(), "how much vertical space the team takes up, while defending");
    }
    else if ((*iter).first.compare("position_offense_width_factor") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "attacking: team width");
      tactics.descriptions.Set((*iter).first.c_str(), "horizontal team width during possession");
    }
    else if ((*iter).first.compare("position_defense_width_factor") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "defending: team width");
      tactics.descriptions.Set((*iter).first.c_str(), "horizontal team width while defending");
    }
    else if ((*iter).first.compare("position_offense_midfieldfocus") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "attacking: midfield joins attack");
      tactics.descriptions.Set((*iter).first.c_str(), "lower values: midfield stays back. higher values: midfield joins attack");
    }
    else if ((*iter).first.compare("position_defense_midfieldfocus") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "defending: midfield stays high up");
      tactics.descriptions.Set((*iter).first.c_str(), "lower values: midfield stays back. higher values: midfield stays higher up");
    }
    else if ((*iter).first.compare("position_offense_sidefocus_strength") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "attacking: forward drive");
      tactics.descriptions.Set((*iter).first.c_str(), "");
    }
    else if ((*iter).first.compare("position_defense_sidefocus_strength") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "defending: backward drive");
      tactics.descriptions.Set((*iter).first.c_str(), "");
    }
    else if ((*iter).first.compare("position_offense_microfocus_strength") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "attacking: compactness around ball");
      tactics.descriptions.Set((*iter).first.c_str(), "");
    }
    else if ((*iter).first.compare("position_defense_microfocus_strength") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "defending: compactness around ball");
      tactics.descriptions.Set((*iter).first.c_str(), "");
    }
    else if ((*iter).first.compare("dribble_offensiveness") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "CPU player on the ball: offensiveness");
      tactics.descriptions.Set((*iter).first.c_str(), "higher values mean more forward drive for the CPU player in possession.");
    }
    else if ((*iter).first.compare("dribble_centermagnet") == 0) {
      tactics.humanReadableNames.Set((*iter).first.c_str(), "CPU player on the ball: prefer center");
      tactics.descriptions.Set((*iter).first.c_str(), "lower values: hug the sidelines more often. higher values: prefer dribbling through the middle of the pitch");
    }

    iter++;
  }
  //printf("result:\n");
  //tactics.userProperties.Print();


  // factory tactics

  tree = loader.Load(factoryTacticsString);

  iter = tree.children.begin();
  while (iter != tree.children.end()) {
    tactics.factoryProperties.Set((*iter).first.c_str(), atof((*iter).second.value.c_str()));
    //printf("value name: %s, value: %f\n", (*iter).first.c_str(), atof((*iter).second.value.c_str()));
    iter++;
  }


  // load players

  std::string order = "formationorder";
  if (national) order = "nationalformationorder";

  result = GetDB()->Query("select id from players where team_id = " + int_to_str(teamDatabaseID) + " or nationalteam_id = " + int_to_str(teamDatabaseID) + " order by " + order);
  for (unsigned int r = 0; r < result->data.size(); r++) {
    //int playerDatabaseID = atoi(playerQuery.result[r * playerQuery.columns + c]);
    int playerDatabaseID = atoi(result->data.at(r).at(0).c_str());
    //printf("loading player %i\n", playerDatabaseID);
    PlayerData *onePlayerData = new PlayerData(playerDatabaseID);
    playerData.push_back(onePlayerData);
  }

  delete result;

}

TeamData::~TeamData() {
  for (int i = 0; i < (signed int)playerData.size(); i++) {
    delete playerData.at(i);
  }
}

FormationEntry TeamData::GetFormationEntry(int num) {
  assert(num >= 0 && num < playerNum);
  return formation[num];
}

void TeamData::SetFormationEntry(int num, FormationEntry entry) {
  formation[num] = entry;
}

void TeamData::SwitchPlayers(int databaseID1, int databaseID2) {
  int index1 = -1;
  int index2 = -1;
  for (int i = 0; i < (signed int)playerData.size(); i++) {
    if (playerData.at(i)->GetDatabaseID() == databaseID1) index1 = i;
    if (playerData.at(i)->GetDatabaseID() == databaseID2) index2 = i;
  }
  assert(index1 != -1);
  assert(index2 != -1);
  //printf("old id on index1 %i\n", playerData.at(index1)->GetDatabaseID());
  //printf("swapping players %i and %i\n", index1, index2);
  PlayerData *tmp = playerData.at(index1);
  playerData.at(index1) = playerData.at(index2);
  playerData.at(index2) = tmp;
  //printf("new id on index1 %i\n", playerData.at(index1)->GetDatabaseID());
}

PlayerData *TeamData::GetPlayerDataByDatabaseID(int id) {
  int index = -1;
  for (int i = 0; i < (signed int)playerData.size(); i++) {
    if (playerData.at(i)->GetDatabaseID() == id) {
      index = i;
      break;
    }
  }
  assert(index != -1);
  return playerData.at(index);
}

void TeamData::SaveLineup() {
}

void TeamData::SaveTactics() {

  const map_Properties *userPropMap = tactics.userProperties.GetProperties();

  map_Properties::const_iterator iter = userPropMap->begin();
  std::string tactics_xml;
  while (iter != userPropMap->end()) {
    const std::string &tacticName = (*iter).first;
    tactics_xml += "<" + tacticName + ">" + real_to_str(tactics.userProperties.GetReal(tacticName.c_str())) + "</" + tacticName + ">\n";
    iter++;
  }

  DatabaseResult *result = GetDB()->Query("update teams set tactics_xml = \"" + tactics_xml + "\" where id = " + int_to_str(GetDatabaseID()) + ";");
  delete result;
}

void TeamData::Save() {
  SaveLineup();
  SaveTactics();
}
