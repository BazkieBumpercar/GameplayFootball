// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_ONTHEPITCH_ELIZACONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_ELIZACONTROLLER

#include "playercontroller.hpp"

#include "../../../gamedefines.hpp"

struct Prerequisites;
class Strategy;
class MentalImage;
class Team;
class Player;

struct PreRating { // for optimization: first calculate everything but passrating, since that one is slow to calculate. cull results before rating pass freedom

  int candidateID;

  float offenseRating; // how close to goal are we?
  float distanceRating; // how far away from possession player are we?
  float movementRating; // prefer continuing in our currect direction
  float formationRating; // prefer movement to our formation position
  float offsideRating; // 0 == offside, 1 == not offside

  float totalRating;
};

bool PreRatingSortFunc(const PreRating &a, const PreRating &b);
bool SortPlayersDeepestFirst(Player *a, Player *b);

class ElizaController : public PlayerController {

  public:
    ElizaController(Match *match);
    virtual ~ElizaController();

    virtual void RequestCommand(PlayerCommandQueue &commandQueue);
    virtual void Process();
    virtual Vector3 GetDirection();
    virtual float GetFloatVelocity();

    void LoadStrategies();

    float GetLazyVelocity(float desiredVelocityFloat);
    Vector3 GetSupportPosition(const MentalImage *mentalImage, const Vector3 &basePosition);
    Vector3 GetSupportPosition_ForceField(const MentalImage *mentalImage, const Vector3 &basePosition, bool makeRun = false);

    virtual void Reset();

  protected:
    void GetOnTheBallCommands(std::vector<PlayerCommand> &commandQueue, Vector3 &rawInputDirection, float &rawInputVelocity);

    void _AddPass(std::vector<PlayerCommand> &commandQueue, Player *target, e_FunctionType passType);
    void _AddPanicPass(std::vector<PlayerCommand> &commandQueue);
    float _GetPassingOdds(Player *targetPlayer, e_FunctionType passType, const std::vector<PlayerImage> &opponentPlayerImages, float ballVelocityMultiplier = 1.0f);
    float _GetPassingOdds(const Vector3 &target, e_FunctionType passType, const std::vector<PlayerImage> &opponentPlayerImages, float ballVelocityMultiplier = 1.0f);
    void _AddCelebration(std::vector<PlayerCommand> &commandQueue);

    Strategy *defenseStrategy;
    Strategy *midfieldStrategy;
    Strategy *offenseStrategy;
    Strategy *goalieStrategy;

    Vector3 lastDesiredDirection;
    float lastDesiredVelocity;

};

#endif
