// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_HUMANOID_UTILS
#define _HPP_HUMANOID_UTILS

#include "../../../gamedefines.hpp"

#include "../../ball.hpp"

#include "base/math/vector3.hpp"
#include "utils/animation.hpp"

using namespace blunted;

struct SpatialState;
class PlayerBase;
class Match;
struct Anim;

e_TouchType GetTouchTypeForBodyPart(const std::string &bodypartname);
unsigned int CalculateTimeNeededToChangeMovement_ms(const Vector3 &currentMovement, const Vector3 &desiredMovement);
float CalculateBiasForFastCornering(const Vector3 &currentMovement, const Vector3 &desiredMovement, float veloPow = 1.0f, float bias = 1.0f);
Vector3 CalculateMovementAtFrame(const std::vector<Vector3> &positions, unsigned int frameNum, unsigned int smoothFrames = 0);
Vector3 GetFrontOfFootOffsetRel(float velocity, radian bodyAngleRel, float height);
bool NeedDefendingMovement(int mySide, const Vector3 &position, const Vector3 &target);
float StretchSprintTo(const float &inputVelocity, float inputSpaceMaxVelocity, float targetMaxVelocity);
void GetDifficultyFactors(Match *match, Player *player, const Vector3 &positionOffset, float &distanceFactor, float &heightFactor, float &ballMovementFactor);
Vector3 GetBallControlVector(Ball *ball, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot, float ffoOffset = 0.0f);
Vector3 GetTrapVector(Match *match, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot);
Vector3 GetShotVector(Match *match, Player *player, const Vector3 &nextStartPos, radian nextStartAngle, radian nextBodyAngle, const Vector3 &outgoingMovement, const Anim *currentAnim, int frameNum, const SpatialState &spatialState, const Vector3 &positionOffset, radian &xRot, radian &yRot, radian &zRot, float autoDirectionBias = 0.0f);

#endif
