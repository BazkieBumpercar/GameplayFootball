// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GAMEDEFINES
#define _HPP_GAMEDEFINES

#include "defines.hpp"

#include "base/math/vector3.hpp"

#include <SDL/SDL.h> // for key ids

using namespace blunted;

extern unsigned long time_ms;

const float idleVelocity = 0.0f;
const float dribbleVelocity = 3.5f;
const float walkVelocity = 5.0f;
const float sprintVelocity = 8.0f;

const float animSprintVelocity = 7.0f;

const float idleDribbleSwitch = 1.8f;
const float dribbleWalkSwitch = 4.2f;
const float walkSprintSwitch = 6.0f;
// PES6 digital control mode, quantizes some input to x degree angles
const bool quantizeDirection = true;

const float analogStickDeadzone = 0.75f;

const float _default_CameraZoom = 0.5f;
const float _default_CameraHeight = 0.3f;
const float _default_CameraFOV = 0.4f;
const float _default_CameraAngleFactor = 0.0f;

const float _default_Difficulty = 0.6f;
const float _default_MatchDuration = 0.4f;

const float _default_QuantizedDirectionBias = 0.0f;

const float _default_AgilityFactor = 0.5f;
const float _default_AccelerationFactor = 0.5f;

const float _default_ShortPass_AutoDirection = 0.4f;
const float _default_ShortPass_AutoPower = 0.7f;
const float _default_ThroughPass_AutoDirection = 0.2f;
const float _default_ThroughPass_AutoPower = 0.7f;
const float _default_HighPass_AutoDirection = 0.2f;
const float _default_HighPass_AutoPower = 0.5f;
const float _default_Shot_AutoDirection = 0.2f;

const float distanceToVelocityMultiplier = 2.6f; // for example: when we need to travel 4 meters, we need to go at velo 4 * distanceToVelocityMultiplier

const unsigned int ballPredictionSize_ms = 3000;
const unsigned int ballHistorySize_ms = 4000;

const float ballDistanceOptimizeThreshold = 10.0f;

const int playerNum = 11;

// how far into an animation the ball is usually touched
const unsigned int defaultTouchOffset_ms = 80;

const float defaultPlayerHeight = 1.92f;

const int temporalSmoother_history_ms = 20;

//#define dataSetSortable 1
#ifdef dataSetSortable
typedef std::list<int> DataSet;
#else
typedef std::deque<int> DataSet;
#endif

const SDLKey defaultKeyIDs[18] = { SDLK_UP, SDLK_RIGHT, SDLK_DOWN, SDLK_LEFT, SDLK_w, SDLK_a, SDLK_s, SDLK_d, SDLK_w, SDLK_a, SDLK_s, SDLK_d, SDLK_q, SDLK_z, SDLK_e, SDLK_c, SDLK_F1, SDLK_RETURN };

class Player;

enum e_Side {
  e_Side_Left,
  e_Side_Right
};

enum e_Velocity {
  e_Velocity_Idle,
  e_Velocity_Dribble,
  e_Velocity_Walk,
  e_Velocity_Sprint
};

enum e_FunctionType {
  e_FunctionType_None,
  e_FunctionType_Movement,
  e_FunctionType_BallControl,
  e_FunctionType_Trap,
  e_FunctionType_ShortPass,
  e_FunctionType_LongPass,
  e_FunctionType_HighPass,
  e_FunctionType_Header,
  e_FunctionType_Shot,
  e_FunctionType_Deflect,
  e_FunctionType_Catch,
  e_FunctionType_Interfere,
  e_FunctionType_Trip,
  e_FunctionType_Sliding,
  e_FunctionType_Special
};

enum e_TouchType {
  e_TouchType_Intentional_Kicked, // goalies can't touch this
  e_TouchType_Intentional_Nonkicked, // headers and such
  e_TouchType_Accidental, // collisions
  e_TouchType_None,
  e_TouchType_SIZE
};

enum e_SetPiece {
  e_SetPiece_None,
  e_SetPiece_KickOff,
  e_SetPiece_GoalKick,
  e_SetPiece_FreeKick,
  e_SetPiece_Corner,
  e_SetPiece_ThrowIn,
  e_SetPiece_Penalty,
};

enum e_MatchPhase {
  e_MatchPhase_PreMatch,
  e_MatchPhase_1stHalf,
  e_MatchPhase_2ndHalf,
  e_MatchPhase_1stExtraTime,
  e_MatchPhase_2ndExtraTime,
  e_MatchPhase_Penalties,
};

enum e_PlayerCommandModifier {
  e_PlayerCommandModifier_None = 0,
  e_PlayerCommandModifier_KnockOn = 1
};

class IController;

struct TouchInfo {

  TouchInfo() {
    inputPower = 0;
    autoDirectionBias = 0;
    autoPowerBias = 0;
    targetPlayer = 0;
    forcedTargetPlayer = 0;
    desiredPower = 0;
  }

  Vector3         inputDirection;
  float           inputPower;

  float           autoDirectionBias;
  float           autoPowerBias;

  Vector3         desiredDirection; // inputdirection after pass function
  float           desiredPower;
  Player          *targetPlayer; // null == do not use
  Player          *forcedTargetPlayer; // null == do not use

};

enum e_StrictMovement {
  e_StrictMovement_False,
  e_StrictMovement_True,
  e_StrictMovement_Dynamic
};

struct PlayerCommand {

  /* specialVar1:

    1: happy celebration
    2: inverse celebration (feeling bad)
    3: referee showing card
  */

  PlayerCommand() {
    desiredFunctionType = e_FunctionType_Movement;
    useDesiredMovement = false;
    desiredVelocityFloat = idleVelocity;
    strictMovement = e_StrictMovement_Dynamic;
    useDesiredLookAt = false;
    useTripType = false;
    useDesiredTripDirection = false;
    onlyDeflectAnimsThatPickupBall = false;
    tripType = 1;
    useSpecialVar1 = false;
    specialVar1 = 0;
    useSpecialVar2 = false;
    specialVar2 = 0;
    modifier = 0;
  }

  e_FunctionType desiredFunctionType;

  bool           useDesiredMovement;
  Vector3        desiredDirection;
  e_StrictMovement strictMovement;

  float          desiredVelocityFloat;

  bool           useDesiredLookAt;
  Vector3        desiredLookAt; // absolute 'look at' position on pitch

  bool           useTouchInfo;
  TouchInfo      touchInfo;

  bool           onlyDeflectAnimsThatPickupBall;

  bool           useTripType;
  int            tripType; // only applicable for trip anims

  bool           useDesiredTripDirection;
  Vector3        desiredTripDirection;

  bool           useSpecialVar1;
  int            specialVar1;
  bool           useSpecialVar2;
  int            specialVar2;

  int            modifier;
};

typedef std::vector<PlayerCommand> PlayerCommandQueue;

enum e_PlayerRole {
  e_PlayerRole_GK,
  e_PlayerRole_CB,
  e_PlayerRole_LB,
  e_PlayerRole_RB,
  e_PlayerRole_DM,
  e_PlayerRole_CM,
  e_PlayerRole_LM,
  e_PlayerRole_RM,
  e_PlayerRole_AM,
  e_PlayerRole_CF,
};

std::string GetRoleName(e_PlayerRole playerRole);
e_PlayerRole GetRoleFromString(const std::string &roleString);

struct FormationEntry {
  e_PlayerRole role;
  Vector3 databasePosition;
  Vector3 position; // adapted to player role (combination of databasePosition and hardcoded role position)
};

struct PlayerImage {
  int teamID;
  signed int side;
  int playerID;
  Player *player;
  Vector3 position;
  Vector3 directionVec;
  Vector3 bodyDirectionVec;
  float velocity;
  Vector3 movement;
  FormationEntry formationEntry;
  FormationEntry dynamicFormationEntry;
};

bool PlayerImageDepthSortFunc(const PlayerImage &a, const PlayerImage &b);

const float pitchHalfW = 55; // only inside side- and backlines
const float pitchHalfH = 36;
const float pitchFullHalfW = 60; // including 'rim'
const float pitchFullHalfH = 40;
const float lineHalfW = 0.06f;

const float goalDepth = 2.55f;
const float goalHeight = 2.5f;
const float goalHalfWidth = 3.7f;

enum e_DecayType {
  e_DecayType_Constant,
  e_DecayType_Variable
};

enum e_MagnetType {
  e_MagnetType_Attract,
  e_MagnetType_Repel
};

// forcefields consist of forcespots, representing a repelling or attracting force from a position, including linearity/etc parameters
struct ForceSpot {
  ForceSpot() {
    exp = 1.0f;
  }
  Vector3 origin;
  e_MagnetType magnetType;
  e_DecayType decayType;
  float exp;
  float power;
  float scale; // scaled #meters until effect is almost decimated
};

class PassRating {

  public:
    PassRating(int playerID, float odds, float pos, float sit) : playerID(playerID), odds(odds), pos(pos), sit(sit), rating(0) {}
    virtual ~PassRating() {}

    void CalculateRating(float opportunism) {
      rating = (sit * 1.0f + odds * 1.0f) * 0.5f * (1 - opportunism) +
               pos * opportunism;
    }

    bool operator < (const PassRating &otherPassRating) const {
      return rating < otherPassRating.rating;
    }

    int playerID;

    // 0 .. 1 == worst .. best
    float odds; // what are the odds a pass to this player will complete?
    float pos; // is this player in a good position?
    float sit; // target's situational rating
    float rating; // resulting rating

};

typedef std::vector<PassRating> PassRatings;


void GetVertexColors(std::map<Vector3, Vector3> &colorCoords);

e_FunctionType StringToFunctionType(const std::string &fun);

float GetGlobalVelocityMultiplier();

#endif
