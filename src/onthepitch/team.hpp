// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TEAM
#define _HPP_TEAM

#include "player/player.hpp"
#include "../data/teamdata.hpp"
#include "teamAIcontroller.hpp"
#include "humangamer.hpp"

class Match;

class Team {

  public:
    Team(int id, Match *match, TeamData *teamData);
    virtual ~Team();

    void Exit();

    void InitPlayers(boost::intrusive_ptr<Node> fullbodyNode, std::map<Vector3, Vector3> &colorCoords);

    Match *GetMatch() { return match; }
    TeamAIController *GetController() { return teamController; }
    boost::intrusive_ptr<Node> GetSceneNode() { return teamNode; }

    int GetID() const { return id; }
    signed int GetSide();
    TeamData *GetTeamData() { return teamData; }

    Player *GetPlayer(int playerID);
    PlayerData *GetPlayerData(int playerID);
    FormationEntry GetFormationEntry(int playerID);
    void SetFormationEntry(int playerID, FormationEntry entry);
    const std::vector<Player*> &GetAllPlayers() { return players; }
    void GetAllPlayers(std::vector<Player*> &allPlayers) { allPlayers.insert(allPlayers.end(), players.begin(), players.end()); }
    void GetActivePlayers(std::vector<Player*> &activePlayers);
    int GetActivePlayerCount() const { return activePlayerCount; }

    unsigned int GetHumanGamerCount() const { return humanGamers.size(); }
    void AddHumanGamer(IHIDevice *hid, e_PlayerColor color);
    void DeleteHumanGamers();
    e_PlayerColor GetPlayerColor(int playerID);
    bool IsHumanControlled(int playerID);

    bool HasPossession() const;
    bool HasUniquePossession() const;
    int GetTimeNeededToGetToBall_ms() const;
    signed int GetBestPossessionPlayerID();
    Player *GetDesignatedTeamPossessionPlayer() { return designatedTeamPossessionPlayer; }
    Player *GetBestPossessionPlayer();
    float GetTeamPossessionAmount() const;
    float GetFadingTeamPossessionAmount() const;
    void SetFadingTeamPossessionAmount(float value);

    void SetLastTouchPlayer(Player *player, e_TouchType touchType = e_TouchType_Intentional_Kicked);
    Player *GetLastTouchPlayer(e_TouchType touchType) const { return lastTouchPlayers[touchType]; }
    Player *GetLastTouchPlayer() const { return lastTouchPlayer; }
    unsigned long GetLastTouchTime_ms() { return lastTouchPlayer ? lastTouchPlayer->GetLastTouchTime_ms() : 0; }
    e_TouchType GetLastTouchType() { return lastTouchType; }
    float GetLastTouchBias(int decay_ms, unsigned long time_ms = 0) { return lastTouchPlayer ? lastTouchPlayer->GetLastTouchBias(decay_ms, time_ms) : 0; }

    void ResetSituation(const Vector3 &focusPos);

    void HumanGamersSelectAnyone();

    void SelectPlayer(Player *player);
    void DeselectPlayer(Player *player);

    void RelaxFatigue(float howMuch);

    void Process();
    void PreparePutBuffers(unsigned long snapshotTime_ms);
    void FetchPutBuffers(unsigned long putTime_ms);
    void Put();
    void Put2D();
    void Hide2D();

    void UpdatePossessionStats();
    void UpdateSwitch();

    Player *GetGoalie();

    void SetKitNumber(int num);

  protected:
    int id;
    Match *match;
    TeamData *teamData;

    bool hasPossession;
    int timeNeededToGetToBall_ms;
    Player *designatedTeamPossessionPlayer;

    float teamPossessionAmount;
    float fadingTeamPossessionAmount;

    TeamAIController *teamController;

    std::vector<Player*> players;
    int activePlayerCount;

    boost::intrusive_ptr<Node> teamNode;
    boost::intrusive_ptr<Node> playerNode;

    std::vector<HumanGamer*> humanGamers;

    // humanGamers index whose turn it is
    // begin() == due next
    std::list<int> switchPriority;

    Player *lastTouchPlayers[e_TouchType_SIZE];
    Player *lastTouchPlayer;
    e_TouchType lastTouchType;

    boost::intrusive_ptr < Resource<Surface> > kit;

};

#endif
