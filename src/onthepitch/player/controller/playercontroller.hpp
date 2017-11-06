// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_ONTHEPITCH_PLAYERCONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_PLAYERCONTROLLER

#include "icontroller.hpp"

class PlayerController : public IController {

  public:
    PlayerController(Match *match);
    virtual ~PlayerController() {};

    virtual void Process();

    virtual void SetPlayer(PlayerBase *player);
    Player *CastPlayer();
    Team *GetTeam() { return team; }
    Team *GetOppTeam() { return oppTeam; }

    const MentalImage *GetMentalImage() { return _mentalImage; }

    virtual int GetReactionTime_ms();

    float GetLastSwitchBias();

    float GetFadingTeamPossessionAmount() { return fadingTeamPossessionAmount; }

    void AddDefensiveComponent(Vector3 &desiredPosition, float bias, int forcedOppID = -1);
    Vector3 GetDefendPosition(Player *opp, float distance = 0.0f);

    virtual void Reset();

  protected:
    float OppBetweenBallAndMeDot();
    float CouldWinABallDuelLikeliness();
    virtual void _Preprocess();
    virtual void _SetInput(const Vector3 &inputDirection, float inputVelocityFloat) { this->inputDirection = inputDirection; this->inputVelocityFloat = inputVelocityFloat; }
    virtual void _KeeperDeflectCommand(PlayerCommandQueue &commandQueue, bool onlyPickupAnims = false);
    virtual void _SetPieceCommand(PlayerCommandQueue &commandQueue);
    virtual void _BallControlCommand(PlayerCommandQueue &commandQueue, bool idleTurnToOpponentGoal = false, bool knockOn = false, bool stickyRunDirection = false, bool keepCurrentBodyDirection = false);
    virtual void _TrapCommand(PlayerCommandQueue &commandQueue, bool idleTurnToOpponentGoal = false, bool knockOn = false);
    virtual void _InterfereCommand(PlayerCommandQueue &commandQueue, bool byAnyMeans = false);
    virtual void _SlidingCommand(PlayerCommandQueue &commandQueue);
    virtual void _MovementCommand(PlayerCommandQueue &commandQueue, bool forceMagnet = false, bool extraHaste = false);

    Vector3 inputDirection;
    float inputVelocityFloat;

    Player *_oppPlayer;
    float _timeNeeded_ms;
    const MentalImage *_mentalImage;

    void _CalculateSituation();

    // only really useful for human gamers, after switching player
    unsigned long lastSwitchTime_ms;
    unsigned int lastSwitchTimeDuration_ms;

    Team *team;
    Team *oppTeam;

    bool hasPossession;
    bool hasUniquePossession;
    bool teamHasPossession;
    bool teamHasUniquePossession;
    bool oppTeamHasPossession;
    bool oppTeamHasUniquePossession;
    bool hasBestPossession;
    bool teamHasBestPossession;
    float possessionAmount;
    float teamPossessionAmount;
    float fadingTeamPossessionAmount;
    int timeNeededToGetToBall;
    int oppTimeNeededToGetToBall;
    bool hasBestChanceOfPossession;
};

#endif
